// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMStunGameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "SOTMStatusTags.h"

USOTMStunGameplayEffect::USOTMStunGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(3.0f));
}

void USOTMStunGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();

	// FindOrAddComponent() ends up calling NewObject() with an implicit NAME_None, which
	// UObject's own constructor-safety check (AssertIfInConstructor) forbids from a constructor.
	// PostInitProperties runs after construction, and gating on the CDO keeps the component
	// singular per class -- non-CDO instances inherit it via normal archetype propagation.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FInheritedTagContainer TagChanges;
		TagChanges.AddTag(TAG_State_Stunned);
		FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(TagChanges);
	}
}
