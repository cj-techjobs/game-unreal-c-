// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMPlayerState.h"
#include "AbilitySystemComponent.h"
#include "SOTMAttributeSetHealth.h"

ASOTMPlayerState::ASOTMPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	HealthAttributeSet = CreateDefaultSubobject<USOTMAttributeSetHealth>(TEXT("HealthAttributeSet"));
}
