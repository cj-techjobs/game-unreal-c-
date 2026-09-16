// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "SOTMAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UBehaviorTree;
struct FAIStimulus;

/** Diagnostic-visible phase of ASOTMAIController's Investigate/Search detour -- BlueprintReadOnly so PIE tests can observe it directly instead of inferring it from position alone. */
UENUM(BlueprintType)
enum class EAISearchPhase : uint8
{
	None,
	MovingToLocation,
	Waiting
};

/**
 * Perception adapter, Blackboard, Behavior Tree, and navigation
 * requests. One controller owns navigation for its pawn.
 *
 * Ported from the original CruelDoll AI Controller Blueprint (BP_AI_Controller),
 * which ran BT_AI on BeginPlay and used a PawnSensing component's OnSeePawn
 * event to set a "seeingPlayer" Blackboard bool. This replaces PawnSensing
 * with AIPerceptionComponent (the engine's supported perception system,
 * per the architecture handbook) and uses its sight sense MaxAge to replace
 * the original's manual 0.5s "StopSeeingPlayer" reset timer.
 *
 * Reacts to State.Stunned (handbook Part 2 section 3.2/4: "Stun tags block
 * relevant abilities and AI execution") by pausing the whole Behavior Tree
 * and stopping movement, rather than teaching every BTTask its own stun
 * check -- there is no BT-graph-editing tool available in this project's
 * toolchain to add a proper Stunned priority branch to the existing BT_AI
 * asset, so PauseLogic/ResumeLogic (a standard, minimal-footprint AIModule
 * mechanism for exactly this "temporarily suspend tree decisions without
 * touching the graph" need) substitutes for it -- functionally equivalent
 * for a single-enemy vertical slice, revisit if BT authoring tooling exists
 * later.
 *
 * Also owns handbook Part 2 section 4.1's perception memory
 * (CurrentVisibleTarget/LastSeenLocation/LastSeenTime/LastHeardLocation/
 * LastHeardTime/Suspicion) and the section 4's Investigate/Search/Return
 * behavior on sight loss or a hearing stimulus -- again implemented as a
 * controller-owned PauseLogic/ResumeLogic detour rather than new BT_AI
 * priority branches, for the same BT-graph-editing-tool reason as Stunned
 * above. TargetEligibility and ScriptedKnowledge are the two section 4.1
 * fields deliberately NOT added: both need systems that don't exist yet
 * (a hiding/safe-zone policy, an authored per-encounter instruction feed)
 * and half-simulating either would be worse than the honest gap -- the
 * same reasoning ASOTMGate's own class comment already applies to the
 * encounter-seal half of its contract.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASOTMAIController();

	virtual void PostInitProperties() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Blackboard key name matching the existing BD_AI blackboard asset. */
	static const FName SeeingPlayerKeyName;

protected:
	/** Behavior tree to run once possession completes. Point this at the existing BT_AI asset on the Blueprint child. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	/**
	 * If true and no BehaviorTreeAsset is set, this controller continuously
	 * moves toward the player every tick instead of running a behavior tree.
	 * For simple always-chasing enemies (e.g. Creation) that never used
	 * perception-gated decisions in the first place. No BT-authoring tool is
	 * available to build one for them, so this is an honest, direct port of
	 * their existing per-tick chase rather than a forced BT conversion.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI")
	bool bAlwaysChasePlayer = false;

	/** Acceptance radius used by bAlwaysChasePlayer's continuous MoveTo. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI", meta = (EditCondition = "bAlwaysChasePlayer"))
	float AlwaysChaseAcceptanceRadius = 5.0f;

	UPROPERTY(VisibleDefaultsOnly, Category = "SOTM|AI")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleDefaultsOnly, Category = "SOTM|AI")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	/** Weak: the sensed actor may be destroyed/streamed out without this controller being told directly. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	TWeakObjectPtr<AActor> CurrentVisibleTarget;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	FVector LastSeenLocation = FVector::ZeroVector;

	/** World seconds of the last confirmed sight stimulus; negative means never seen. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	float LastSeenTime = -1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	FVector LastHeardLocation = FVector::ZeroVector;

	/** World seconds of the last hearing stimulus; negative means never heard. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	float LastHeardTime = -1.0f;

	/** Bounded 0..1 awareness. Sight pushes it to 1 while visible; hearing nudges it; it decays back toward 0 each tick when neither sense is currently active. Authored policy (which behaviors read it, thresholds) is future work -- only the bounded fact is owned here for now. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	float Suspicion = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI|Perception")
	float SuspicionDecayPerSecond = 0.5f;

	/** How close MoveToLocation must get during Investigate/Search before it counts as arrived. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI|Perception")
	float InvestigateAcceptanceRadius = 80.0f;

	/** How long to wait at the investigated location before returning control to the Behavior Tree. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|AI|Perception")
	float SearchWaitSeconds = 2.5f;

	/** Diagnostic-visible phase of the current Investigate/Search detour, if any. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|AI|Perception")
	EAISearchPhase CurrentSearchPhase = EAISearchPhase::None;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	/** Bound to the possessed pawn's ASC (if any) for State.Stunned; unbound in OnUnPossess. */
	FDelegateHandle StunTagEventHandle;

	void HandleStunTagChanged(const FGameplayTag Tag, int32 NewCount);

	FTimerHandle SearchWaitTimerHandle;

	/** Pauses BT logic and moves toward Location; restarts if already investigating. No-op while stunned -- HandleStunTagChanged owns the pause in that case, and moving would contradict "stunned". */
	void BeginInvestigate(const FVector& Location);

	UFUNCTION()
	void HandleInvestigateMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	void HandleSearchWaitComplete();

	/** Clears search state and resumes BT logic, unless something else (e.g. a still-active stun) should keep it paused. */
	void EndInvestigate();

	bool IsCurrentlyStunned() const;
};
