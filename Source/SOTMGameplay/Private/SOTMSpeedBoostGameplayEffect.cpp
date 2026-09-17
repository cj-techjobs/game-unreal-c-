// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMSpeedBoostGameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "SOTMStatusTags.h"

USOTMSpeedBoostGameplayEffect::USOTMSpeedBoostGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));
}

void USOTMSpeedBoostGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();

	// See USOTMStunGameplayEffect::PostInitProperties for why this is here
	// (FindOrAddComponent's NewObject call is constructor-unsafe) and why
	// it's gated on the CDO.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FInheritedTagContainer TagChanges;
		TagChanges.AddTag(TAG_State_SpeedBoosted);
		FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(TagChanges);
	}
}
