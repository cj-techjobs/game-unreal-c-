// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "SOTMSpeedBoostCooldownGameplayEffect.generated.h"

/**
 * USOTMSpeedBoostGameplayAbility's CooldownGameplayEffectClass. Grants
 * Cooldown.SpeedBoost for a fixed duration; GAS's own
 * UGameplayAbility::CommitAbility/GetCooldownTags machinery applies and
 * checks it automatically (AbilityDefinition.h's own comment: the ability
 * CDO's CooldownGameplayEffectClass is "GAS's own canonical source" for
 * cooldown timing) -- no hand-rolled cooldown logic needed. Structurally
 * identical to USOTMStunGameplayEffect/USOTMSpeedBoostGameplayEffect; kept
 * as its own tiny class rather than a shared generic "cooldown GE" because
 * the granted tag is part of each cooldown's identity and must not
 * cross-contaminate between abilities.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMSpeedBoostCooldownGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USOTMSpeedBoostCooldownGameplayEffect();

protected:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};
