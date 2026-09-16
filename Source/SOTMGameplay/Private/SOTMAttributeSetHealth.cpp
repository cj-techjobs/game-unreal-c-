// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMAttributeSetHealth.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

USOTMAttributeSetHealth::USOTMAttributeSetHealth()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
}

void USOTMAttributeSetHealth::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USOTMAttributeSetHealth, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USOTMAttributeSetHealth, MaxHealth, COND_None, REPNOTIFY_Always);
}

void USOTMAttributeSetHealth::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void USOTMAttributeSetHealth::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute != GetHealthAttribute())
	{
		return;
	}

	const float ClampedHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
	SetHealth(ClampedHealth);

	const bool bIsAboveZeroNow = ClampedHealth > 0.0f;
	if (bWasAboveZero && !bIsAboveZeroNow)
	{
		OnHealthDepleted.Broadcast(Data.Target.GetAvatarActor());
	}
	bWasAboveZero = bIsAboveZeroNow;
}

void USOTMAttributeSetHealth::ResetForRespawn()
{
	SetHealth(GetMaxHealth());
	bWasAboveZero = true;
}

void USOTMAttributeSetHealth::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USOTMAttributeSetHealth, Health, OldValue);
}

void USOTMAttributeSetHealth::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USOTMAttributeSetHealth, MaxHealth, OldValue);
}
