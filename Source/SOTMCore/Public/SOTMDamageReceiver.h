// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "SOTMDamageReceiver.generated.h"

UINTERFACE(MinimalAPI)
class USOTMDamageReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * Damage capability for non-GAS environmental receivers only (e.g. a
 * breakable prop). GAS actors (player, enemies) use one damage/effect
 * pipeline instead of this interface.
 */
class SOTMCORE_API ISOTMDamageReceiver
{
	GENERATED_BODY()

public:
	virtual void ReceiveEnvironmentalDamage(float Amount, const FGameplayTag& DamageType, AActor* Instigator) = 0;
};
