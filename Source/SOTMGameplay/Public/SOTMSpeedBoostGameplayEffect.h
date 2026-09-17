// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "SOTMSpeedBoostGameplayEffect.generated.h"

/**
 * Grants State.SpeedBoosted to its target for a fixed duration (handbook
 * Part 2 section 3.1: Speed Boost "applies a timed movement multiplier") --
 * no attribute modifier, just a target tag for the duration policy to
 * expire naturally through GAS's own active-effect timer, mirroring
 * USOTMStunGameplayEffect's established pattern exactly. The actual speed
 * change is applied separately by USOTMSpeedBoostGameplayAbility calling
 * USOTMMovementPolicyComponent::SetSpeedBoostMultiplier -- this effect only
 * supplies the observable State.SpeedBoosted fact (anti-restack guard,
 * future HUD/AI hooks), matching Stun's own division between "the tag" and
 * "what reacts to the tag."
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMSpeedBoostGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USOTMSpeedBoostGameplayEffect();

protected:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};
