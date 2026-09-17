// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Enhanced Input ability slots (handbook Part 2 section 1's "ASC input
 * adapter": "IA_AbilityPrimary / IA_AbilitySecondary -- Slot intent resolved
 * to owned ability ID"). Matched against UAbilityDefinition::InputTag
 * (stamped onto each granted FGameplayAbilitySpec's dynamic tags by
 * UProgressionSubsystem::ReconcileAbilityGrants) so
 * ASOTMPlayerCharacter::TryActivateAbilityByInputTag can resolve a pressed
 * input slot to whichever ability was granted for it, without either side
 * hardcoding a specific ability class. Native tags, not
 * DefaultGameplayTags.ini content -- code constants both the input handler
 * and the grant registry branch on directly, same reasoning as
 * SOTMStatusTags.h/SOTMDamageTags.h. Manual SOTMCORE_API extern (not the
 * bare UE_DECLARE_GAMEPLAY_TAG_EXTERN macro) so it resolves correctly from
 * SOTMGameplay across the module boundary.
 */
extern SOTMCORE_API FNativeGameplayTag TAG_Input_AbilityPrimary;
extern SOTMCORE_API FNativeGameplayTag TAG_Input_AbilitySecondary;
