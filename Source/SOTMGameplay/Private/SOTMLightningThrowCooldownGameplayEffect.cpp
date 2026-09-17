// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMLightningThrowCooldownGameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "SOTMCooldownTags.h"

USOTMLightningThrowCooldownGameplayEffect::USOTMLightningThrowCooldownGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(4.0f));
}

void USOTMLightningThrowCooldownGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FInheritedTagContainer TagChanges;
		TagChanges.AddTag(TAG_Cooldown_LightningThrow);
		FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(TagChanges);
	}
}
