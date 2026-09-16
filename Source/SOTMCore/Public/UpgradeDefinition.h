// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UpgradeDefinition.generated.h"

class UTexture2D;

/**
 * Granted ability/effect identity, ranks, price, prerequisites, and station
 * category for a purchasable upgrade (handbook Part 1 sections 6.3 and 6.4
 * -- 6.4 gives this exact class as its illustrative reflected-contract
 * example). Mirrors that example's ContentId/DisplayName/MaxRank/Icon
 * fields, except ContentId is EditDefaultsOnly here rather than the
 * illustrative VisibleAnywhere: no content-factory tool exists yet to
 * stamp it in some other way, so a human has to be able to type it in (the
 * same deviation already made for UChapterDefinition/UMovementDefinition).
 *
 * GrantedAbilityId is a tag placeholder, not a UAbilityDefinition reference
 * -- that type and GAS integration don't exist yet. PriceRowName is a flat
 * FlatPrice instead of an actual DataTable row reference for the same
 * reason: no shared price DataTable exists yet, and inventing one for a
 * single field would be the mirrored-value problem section 6.3 itself
 * warns against. PrerequisiteContentIds/StationCategory are authoring data
 * only -- no purchase command, wallet integration, or prerequisite
 * validation reads them yet (handbook Part 2 section 7's TryPurchaseUpgrade
 * is a separate, larger increment: UProgressionSubsystem doesn't track
 * owned ranks at all yet).
 */
UCLASS(BlueprintType)
class SOTMCORE_API UUpgradeDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ContentId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	int32 SchemaVersion = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer OwnerTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxRank = 1;

	/** Which ability/effect this upgrade grants. A tag placeholder until UAbilityDefinition/GAS exist to resolve it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	FGameplayTag GrantedAbilityId;

	/** Flat price in the single configured currency (handbook Part 2 section 7: "Names such as coins, fragments... are localized presentation choices for the same configured currency ID"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0"))
	int64 FlatPrice = 0;

	/** Other upgrades' ContentIds that must already be owned before this one may be purchased. Not yet validated by any subsystem -- authoring data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	TArray<FName> PrerequisiteContentIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	FGameplayTag StationCategory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Authoring", meta = (MultiLine = "true"))
	FString EditorNotes;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return ContentId.IsNone() ? FPrimaryAssetId()
			: FPrimaryAssetId(FPrimaryAssetType(TEXT("Upgrade")), ContentId);
	}
};
