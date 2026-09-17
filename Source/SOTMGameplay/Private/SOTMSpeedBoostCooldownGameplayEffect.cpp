// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMSpeedBoostCooldownGameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "SOTMCooldownTags.h"

USOTMSpeedBoostCooldownGameplayEffect::USOTMSpeedBoostCooldownGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(8.0f));
}

void USOTMSpeedBoostCooldownGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FInheritedTagContainer TagChanges;
		TagChanges.AddTag(TAG_Cooldown_SpeedBoost);
		FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(TagChanges);
	}
}
