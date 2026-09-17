// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Per-ability cooldown tags granted by each ability's native
 * CooldownGameplayEffectClass (handbook Part 2 section 3: "cooldowns" is
 * one of the GAS-owned concerns; AbilityDefinition.h's own comment notes
 * the ability CDO's CooldownGameplayEffectClass is GAS's canonical source
 * for cooldown timing -- this file only supplies the granted-tag identity
 * each cooldown GE needs, one tag per ability so cooldowns never
 * cross-contaminate). Native tags, not DefaultGameplayTags.ini content,
 * same reasoning as SOTMStatusTags.h. Manual SOTMCORE_API extern (not the
 * bare UE_DECLARE_GAMEPLAY_TAG_EXTERN macro) so it resolves correctly from
 * SOTMGameplay across the module boundary.
 */
extern SOTMCORE_API FNativeGameplayTag TAG_Cooldown_SpeedBoost;
extern SOTMCORE_API FNativeGameplayTag TAG_Cooldown_LightningThrow;
