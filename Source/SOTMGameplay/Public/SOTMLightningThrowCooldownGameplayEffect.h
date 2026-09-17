// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "SOTMLightningThrowCooldownGameplayEffect.generated.h"

/**
 * USOTMLightningThrowGameplayAbility's CooldownGameplayEffectClass. Grants
 * Cooldown.LightningThrow for a fixed duration. See
 * USOTMSpeedBoostCooldownGameplayEffect for the full rationale (same
 * pattern, distinct tag per ability).
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMLightningThrowCooldownGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USOTMLightningThrowCooldownGameplayEffect();

protected:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};
