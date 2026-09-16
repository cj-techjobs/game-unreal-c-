// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Character status tags that ability activation and AI decision-making
 * branch on directly (handbook Part 2 section 3.2: "Stun tags block
 * relevant abilities and AI execution"). Native tags, not
 * DefaultGameplayTags.ini content, for the same reason as
 * SOTMResultTags.h/SOTMDamageTags.h -- code constants every branch reads,
 * not designer-authored data. Manual SOTMCORE_API extern (not the bare
 * UE_DECLARE_GAMEPLAY_TAG_EXTERN macro) so it resolves correctly from
 * SOTMGameplay across the module boundary.
 */
extern SOTMCORE_API FNativeGameplayTag TAG_State_Stunned;
