// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "SOTMDamageGameplayEffect.generated.h"

/**
 * Instant Health reduction via a single SetByCaller (Data.Damage) modifier
 * (handbook Part 2 section 3.2: "All gameplay damage enters one pipeline...
 * effect calculation -> health change"). Configured entirely in the
 * constructor rather than as Blueprint/asset data, per this project's
 * "logic in C++" preference and matching Epic's own convention for simple
 * native GameplayEffect classes (e.g. the ActionRPG sample's damage GE).
 *
 * No mitigation/resistance calculation yet -- callers set the final damage
 * magnitude directly. An ExecutionCalculation class is the natural upgrade
 * if armor/resistance math is needed later; not built now since nothing
 * needs it yet.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMDamageGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	USOTMDamageGameplayEffect();
};
