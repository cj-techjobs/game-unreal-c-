// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "SOTMIdentityTypes.h"
#include "SOTMCommandTypes.h"
#include "GameFlowSubsystem.generated.h"

/** Collapses the handbook's Part 1 section 5 flowchart (Boot/Frontend/Select/Load/Prepare/Pawn/Ready/Play/Transition) into states this increment can actually back with real behavior -- Select/Load/Prepare/Pawn/Ready have no Asset Manager bundle loading or ASC rebuild yet to distinguish, so they're all just "Loading". */
UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
	Boot,
	Frontend,
	Loading,
	Playing,
	ShuttingDown
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameFlowStateChanged, EGameFlowState, NewState);

/** What kind of terminal outcome a queued candidate represents (handbook Part 2 section 3.2's "a valid chapter-victory candidate takes priority over a simultaneous player-death candidate"). */
UENUM(BlueprintType)
enum class ETerminalCandidateKind : uint8
{
	PlayerDeath,
	ChapterVictory
};

/** Internal-only queue entry; not BlueprintType since nothing outside this subsystem needs to construct or read one -- callers only ever see the CandidateId returned by TryAcceptDeath/TryAcceptVictory and the result delivered via OnTerminalCandidateResolved. */
USTRUCT()
struct FSOTMTerminalCandidate
{
	GENERATED_BODY()

	UPROPERTY()
	FGuid CandidateId;

	UPROPERTY()
	ETerminalCandidateKind Kind = ETerminalCandidateKind::PlayerDeath;
};

/** Kind is the resolved candidate's own kind (not necessarily the batch's overall winner) -- a listener that only cares about one kind (e.g. a game mode driving respawn only for PlayerDeath) can filter on it directly instead of separately tracking which CandidateIds it personally raised. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTerminalCandidateResolved, FGuid, CandidateId, bool, bAccepted, ETerminalCandidateKind, Kind);

/**
 * Application flow state, world-generation counter, and accepted-death-ID
 * de-duplication. Deliberately does not perform actual level travel
 * (UGameplayStatics::OpenLevel) or Asset Manager bundle loading itself --
 * this owns the state/generation bookkeeping around those (handbook Part 1
 * section 5 steps 3 and 8: "increments world generation, cancels old
 * requests" / "Confirm mandatory requirements... enter Playing"), while the
 * actual travel call and readiness wait (ChapterSubsystem::IsReady) stay
 * with their existing owners.
 *
 * Also implements the handbook Part 2 section 3.2 terminal-resolution
 * phase: "Queue terminal candidates rather than committing deaths/
 * victories inside hit callbacks." TryAcceptDeath/TryAcceptVictory only
 * enqueue and return a CandidateId (or an invalid Id if rejected outright
 * without ever queuing); FTickableGameObject::Tick is the resolution
 * phase, running once per frame in the engine's dedicated tickable-object
 * pass (after actor ticks, before rendering) rather than synchronously
 * inside whatever call stack queued the candidate -- exactly the
 * separation section 3.2 asks for, without needing a bespoke phase
 * scheduler. A batch is resolved against a snapshot taken at that Tick:
 * a ChapterVictory candidate beats any simultaneous PlayerDeath candidates;
 * otherwise the first-queued PlayerDeath is accepted. Once a terminal flow
 * is accepted, every later candidate (this batch or any future one) is
 * rejected via OnTerminalCandidateResolved without re-evaluating priority.
 *
 * TryAcceptVictory's "accept" only marks the terminal flow locked to
 * victory -- it does not call into UChapterSubsystem or commit any
 * chapter-specific completion state, since that system's real completion
 * contract doesn't exist yet. This keeps the priority rule itself real and
 * exercisable (both candidate kinds can actually be produced and raced)
 * without inventing what a full chapter-victory commit should mutate.
 * The exact priority ordering implemented here matches the handbook's
 * text, but the handbook itself flags this as needing to be "tested and
 * approved with the boss design" -- treat it as the documented starting
 * point, not a closed design decision.
 */
UCLASS()
class SOTMGAMEPLAY_API UGameFlowSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * FTickableGameObject's own header warns not to rely on its constructor
	 * to start ticking a UObject: construction/destruction can happen at
	 * unsafe times (e.g. off the game thread), so registration defaults to
	 * Never here and Initialize/Deinitialize explicitly flip
	 * SetTickableTickType(Always/Never) instead -- the only game-thread-safe
	 * window for a USubsystem.
	 */
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Never; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGameFlowSubsystem, STATGROUP_Tickables); }

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Flow")
	FOnTerminalCandidateResolved OnTerminalCandidateResolved;
	UPROPERTY(BlueprintAssignable, Category = "SOTM|Flow")
	FOnGameFlowStateChanged OnGameFlowStateChanged;

	UFUNCTION(BlueprintPure, Category = "SOTM|Flow")
	EGameFlowState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Flow")
	FWorldGeneration GetCurrentWorldGeneration() const { return CurrentGeneration; }

	/** An async operation should capture GetCurrentWorldGeneration() when it starts and check this on completion, discarding a stale (no-longer-current) result. */
	UFUNCTION(BlueprintPure, Category = "SOTM|Flow")
	bool IsGenerationCurrent(FWorldGeneration Generation) const { return Generation == CurrentGeneration; }

	/** Enters Loading and bumps the world generation, invalidating in-flight async work from the previous generation. Fails only if already ShuttingDown. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	FCommandResult BeginLoading();

	/** Enters Playing once the caller has confirmed readiness (e.g. via ChapterSubsystem::IsReady()). Fails if not currently Loading. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	FCommandResult EnterPlaying();

	/** No-op if already ShuttingDown. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	void EnterFrontend();

	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	void BeginShutdown();

	/**
	 * Queues a player-death candidate for the next terminal-resolution
	 * phase rather than committing immediately -- see the class comment.
	 * De-duplicates by DeathEventId at queue time (e.g. two damage sources
	 * killing the player on the same frame produce one candidate, not two),
	 * and rejects outright (returning an invalid FGuid, never queuing) if
	 * DeathEventId is invalid/already seen, flow isn't currently Playing,
	 * or a terminal flow was already accepted. A valid returned Id means
	 * "queued", not "accepted" -- wait for OnTerminalCandidateResolved with
	 * that same Id for the real answer.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	FGuid TryAcceptDeath(const FGuid& DeathEventId);

	/**
	 * Queues a chapter-victory candidate for the next terminal-resolution
	 * phase. See the class comment for what "accepted" does and doesn't
	 * commit. Rejects outright if flow isn't currently Playing or a
	 * terminal flow was already accepted.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Flow")
	FGuid TryAcceptVictory();

private:
	UPROPERTY()
	EGameFlowState CurrentState = EGameFlowState::Boot;

	UPROPERTY()
	FWorldGeneration CurrentGeneration;

	/** DeathEventIds already queued (whether their candidate was later accepted or rejected) -- see TryAcceptDeath's comment on why dedup happens at queue time now, not accept time. */
	UPROPERTY()
	TSet<FGuid> SeenDeathEventIds;

	UPROPERTY()
	TArray<FSOTMTerminalCandidate> PendingCandidates;

	UPROPERTY()
	bool bTerminalFlowAccepted = false;

	void SetState(EGameFlowState NewState);
	void ResolvePendingCandidates();
};
