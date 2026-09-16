// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ObjectiveDefinition.generated.h"

/** Handbook Part 2 section 8's "bounded condition library" -- deliberately closed, not a string expression interpreter. */
UENUM(BlueprintType)
enum class EObjectiveConditionType : uint8
{
	EnterArea,
	CollectCurrencyTotal,
	HaveCurrency,
	OwnUpgrade,
	HaveItemOrWorldFact,
	CompleteEncounter,
	ReachMilestone,
	AttemptCounter
};

/** Handbook Part 2 section 8.1: persistent facts vs. attempt-scoped counters that reset with a checkpoint retry. */
UENUM(BlueprintType)
enum class EObjectiveScope : uint8
{
	Persistent,
	EncounterScoped
};

/**
 * Typed condition and dependencies for one objective (handbook Part 1
 * section 6.3 / Part 2 section 8). UObjectiveSubsystem evaluates transitions
 * pushed in by callers today; it does not yet evaluate ConditionType/
 * ConditionTargetTag against live world state (that "evaluate world inputs
 * into transitions" half needs marker actors, an inventory, and per-
 * condition-type readers that don't exist yet -- see UObjectiveSubsystem's
 * own class comment). This asset exists so that data can be authored now:
 * ActivateObjectiveFromDefinition already reads ObjectiveId/ProgressTarget.
 *
 * SuccessorObjectiveIds and OutcomeIds are declarative data only -- nothing
 * auto-activates a successor on completion yet, matching how
 * UChapterDefinition's ObjectiveRootIds/TerminalOutcomeId were introduced
 * before anything consumed them beyond the one wiring point that existed.
 */
UCLASS(BlueprintType)
class SOTMCORE_API UObjectiveDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Set once at authoring time; changes require a save migration. Asset Manager identity -- see GetPrimaryAssetId(). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ContentId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	int32 SchemaVersion = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer OwnerTags;

	/** Runtime semantic identity -- what UObjectiveSubsystem tracks state under. Distinct from ContentId, the Asset Manager/content-addressing identity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag ObjectiveId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	/** Other objectives that must already be Completed before this one may be activated. Not yet validated by any subsystem -- authoring data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	TArray<FGameplayTag> PrerequisiteObjectiveIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	EObjectiveConditionType ConditionType = EObjectiveConditionType::ReachMilestone;

	/** The condition's single semantic parameter: which area/upgrade/encounter/milestone/fact, depending on ConditionType. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	FGameplayTag ConditionTargetTag;

	/** Passed as ActivateObjective's Target -- how many progress units complete this objective (1 for a binary condition). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "1", UIMin = "1"))
	int32 ProgressTarget = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	EObjectiveScope Scope = EObjectiveScope::Persistent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	bool bIsOptional = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	bool bCanFail = true;

	/** Objectives to consider activating once this one completes. Declarative only -- see class comment. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outcomes")
	TArray<FGameplayTag> SuccessorObjectiveIds;

	/** Declarative only -- the "idempotent outcome transaction" system (handbook Part 2 section 8.1) doesn't exist yet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outcomes")
	TArray<FGameplayTag> OutcomeIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	FGameplayTag MarkerId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TArray<FGameplayTag> HintIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Authoring", meta = (MultiLine = "true"))
	FString EditorNotes;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return ContentId.IsNone() ? FPrimaryAssetId()
			: FPrimaryAssetId(FPrimaryAssetType(TEXT("Objective")), ContentId);
	}
};
