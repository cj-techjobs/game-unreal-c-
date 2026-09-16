// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "SOTMStunGameplayEffect.generated.h"

/**
 * Grants State.Stunned to its target for a fixed duration (handbook Part 2
 * section 3.1: "Lightning's default cousin effect is stun, not damage") --
 * no attribute modifier, just a target tag for the duration policy to
 * expire naturally through GAS's own active-effect timer. Cousin AI reacts
 * to this tag via ASOTMAIController's tag-changed subscription
 * (Pause/ResumeLogic), not a bespoke per-task stun check, so any future
 * stun source (not just Lightning) gets the same reaction for free.
 * Configured entirely in the constructor, matching
 * USOTMDamageGameplayEffect's established pattern for simple native GEs.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMStunGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USOTMStunGameplayEffect();

protected:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
};
