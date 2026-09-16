// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "AbilityDefinition.generated.h"

UENUM(BlueprintType)
enum class ESOTMAbilityTargetPolicy : uint8
{
	Self,
	SingleTarget,
	Projectile,
	AreaOfEffect
};

/**
 * GAS implementation reference, input tag, level, target policy, and cue
 * tags for a grantable ability (handbook Part 1 section 4's UAbilityDefinition
 * row, Part 2 section 3's ability grant registry).
 *
 * Deliberately has no separate cost/cooldown fields: those live on
 * AbilityClass's own CDO as CostGameplayEffectClass/CooldownGameplayEffectClass,
 * which is GAS's own canonical source for them. Duplicating numbers here would
 * be the same mirrored-value problem UUpgradeDefinition's own comments already
 * warn against for price data -- AbilityLevel is the only genuinely
 * definition-level tuning value GAS abilities take generically.
 */
UCLASS(BlueprintType)
class SOTMCORE_API UAbilityDefinition : public UPrimaryDataAsset
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

	/** The GAS implementation this definition grants. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Enhanced Input slot this ability responds to once bound (handbook Part 2 section 1's ASC input adapter, e.g. IA_AbilityPrimary). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tuning", meta = (ClampMin = "1", UIMin = "1"))
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tuning")
	ESOTMAbilityTargetPolicy TargetPolicy = ESOTMAbilityTargetPolicy::Self;

	/** Gameplay cue tags this ability is expected to invoke; authoring/documentation only until a cue library exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	FGameplayTagContainer CueTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Authoring", meta = (MultiLine = "true"))
	FString EditorNotes;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return ContentId.IsNone() ? FPrimaryAssetId()
			: FPrimaryAssetId(FPrimaryAssetType(TEXT("Ability")), ContentId);
	}
};
