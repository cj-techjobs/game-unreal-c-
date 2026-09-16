// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SOTMObjectiveTypes.h"
#include "SOTMCommandTypes.h"
#include "ObjectiveSubsystem.generated.h"

class UObjectiveDefinition;
class UProgressionSubsystem;
struct FProgressionSnapshot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveTransitioned, const FObjectiveState&, State);

/**
 * Tracks objective activation/progress/completion, keyed by a FGameplayTag
 * identity (matching ISOTMObjectiveTarget's marker id). Gameplay-only:
 * excluded from editor preview worlds so it never activates outside
 * Game/PIE.
 *
 * Now evaluates three of the handbook's Part 2 section 8 bounded condition
 * library entries against live state, event-driven rather than polled (no
 * global actor scans, per section 16's performance rule):
 *   - EnterArea: ASOTMObjectiveMarker actors call ReportAreaEntered on
 *     player overlap; matched against active objectives by ConditionTargetTag.
 *   - HaveCurrency: re-checked against UProgressionSubsystem's wallet
 *     whenever OnProgressionChanged fires.
 *   - OwnUpgrade: re-checked against OwnedUpgradeRanks the same way.
 *     ConditionTargetTag's registered tag NAME must exactly match the
 *     target UUpgradeDefinition's ContentId string -- there's no other
 *     link between the two ID systems (GameplayTag vs FName) without
 *     inventing a lookup table for one field.
 * CollectCurrencyTotal, HaveItemOrWorldFact, CompleteEncounter,
 * ReachMilestone, and AttemptCounter remain unevaluated: each needs a
 * system that doesn't exist yet (a distinct collected-total ledger and
 * TryCollectPickup, an inventory, encounter outcomes, a story-outcome
 * ledger, and attempt-scoped counters, respectively) -- half-simulating
 * them without that backing data would be worse than leaving them as
 * explicit push-in-only conditions for now.
 *
 * Purely in-memory for this increment -- not yet part of the save
 * envelope, matching FProgressionSnapshot/SOTMSaveGame's own deferral of
 * ObjectiveRecords until this subsystem has a real record shape to persist.
 */
UCLASS()
class SOTMGAMEPLAY_API UObjectiveSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Objective")
	FOnObjectiveTransitioned OnObjectiveTransitioned;

	/** Called by ASOTMObjectiveMarker (or any other EnterArea source) on a valid player overlap. Advances every active objective whose ConditionType is EnterArea and ConditionTargetTag matches MarkerId. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	void ReportAreaEntered(FGameplayTag MarkerId);

	/** Fails if ObjectiveId is invalid, Target < 1, or the objective is already Active/Completed. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	FCommandResult ActivateObjective(FGameplayTag ObjectiveId, int32 Target = 1);

	/** Convenience over ActivateObjective: reads ObjectiveId/ProgressTarget from Definition and remembers it for GetObjectiveDefinition. No-op if Definition is null. Does not evaluate ConditionType against world state -- see class comment. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	FCommandResult ActivateObjectiveFromDefinition(UObjectiveDefinition* Definition);

	/** Null unless ObjectiveId was activated via ActivateObjectiveFromDefinition. */
	UFUNCTION(BlueprintPure, Category = "SOTM|Objective")
	UObjectiveDefinition* GetObjectiveDefinition(FGameplayTag ObjectiveId) const;

	/** Fails if the objective isn't Active or Amount isn't positive. Auto-completes once Progress reaches Target. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	FCommandResult AdvanceObjective(FGameplayTag ObjectiveId, int32 Amount);

	/** For binary (non-progress) objectives, e.g. EnterArea/CompleteEncounter. Fails if not Active. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	FCommandResult CompleteObjective(FGameplayTag ObjectiveId);

	/** Fails if not Active. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	FCommandResult FailObjective(FGameplayTag ObjectiveId);

	/** Returns a default (Inactive, Progress 0) state if ObjectiveId was never activated. */
	UFUNCTION(BlueprintPure, Category = "SOTM|Objective")
	FObjectiveState GetObjectiveState(FGameplayTag ObjectiveId) const;

	UFUNCTION(BlueprintPure, Category = "SOTM|Objective")
	TArray<FObjectiveState> GetActiveObjectives() const;

	/** Resets any Active/Failed objective back to Inactive with zero progress; Completed objectives are left untouched. For checkpoint restoration, standing in for a real encounter-scope/persistent-scope split until FObjectiveState gains a scope field. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Objective")
	void ResetAllTransientState();

private:
	UPROPERTY()
	TMap<FGameplayTag, FObjectiveState> ObjectiveStates;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UObjectiveDefinition>> DefinitionsByObjectiveId;

	void Transition(const FObjectiveState& State);

	UFUNCTION()
	void HandleProgressionChanged(const FProgressionSnapshot& Snapshot);
};
