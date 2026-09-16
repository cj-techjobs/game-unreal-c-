// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ChapterDefinition.generated.h"

class UWorld;

/**
 * Chapter identity, map, entry checkpoint, and objective roots (handbook
 * Part 1 section 6.3's UChapterDefinition row). Deliberately omits
 * required-asset-bundle lists, initial ability grants, run policy, and
 * build availability -- those need Asset Manager bundle plumbing,
 * UAbilityDefinition/GAS, and UBuildProfileDefinition, none of which exist
 * yet. Registered as the "Chapter" primary asset type in DefaultGame.ini,
 * scanning /Game/SOTM/Chapters (handbook Part 1 section 6.5).
 *
 * Follows the exact ContentId/GetPrimaryAssetId pattern from the
 * handbook's own Part 1 section 6.4 UUpgradeDefinition example, rather
 * than inventing a different identity scheme.
 */
UCLASS(BlueprintType)
class SOTMGAMEPLAY_API UChapterDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Set once at authoring time; changes require a save migration. Asset Manager identity -- see GetPrimaryAssetId(). EditDefaultsOnly rather than VisibleAnywhere because no content-factory tool exists yet to stamp this in some other way -- a human (or an equivalent authoring tool, once one exists) has to be able to type it in. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ContentId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	int32 SchemaVersion = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer OwnerTags;

	/** Runtime semantic identity -- what ChapterSubsystem::ActivateChapter tracks as "the active chapter". Distinct from ContentId, which is the Asset Manager/content-addressing identity. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTag ChapterId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content")
	TSoftObjectPtr<UWorld> Map;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content")
	FGameplayTag EntryCheckpointId;

	/** Objectives to activate (via UObjectiveSubsystem::ActivateObjective) when this chapter starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content")
	TArray<FGameplayTag> ObjectiveRootIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Content")
	FGameplayTag TerminalOutcomeId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Authoring", meta = (MultiLine = "true"))
	FString EditorNotes;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return ContentId.IsNone() ? FPrimaryAssetId()
			: FPrimaryAssetId(FPrimaryAssetType(TEXT("Chapter")), ContentId);
	}
};
