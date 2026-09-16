// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMDamageGameplayEffect.h"
#include "SOTMAttributeSetHealth.h"
#include "SOTMDamageTags.h"

USOTMDamageGameplayEffect::USOTMDamageGameplayEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo HealthModifier;
	HealthModifier.Attribute = USOTMAttributeSetHealth::GetHealthAttribute();
	HealthModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = TAG_Data_Damage;
	HealthModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(HealthModifier);
}
