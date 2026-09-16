// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * SetByCaller data tag for USOTMDamageGameplayEffect's Health modifier
 * (handbook Part 2 section 3.2: "All gameplay damage enters one pipeline").
 * A code constant every damage-applying call site sets by tag, not
 * designer-authored content -- same reasoning as SOTMResultTags.h for why
 * this is a native tag (manual SOTMCORE_API extern, not the bare
 * UE_DECLARE_GAMEPLAY_TAG_EXTERN macro) rather than a DefaultGameplayTags.ini
 * entry.
 */
extern SOTMCORE_API FNativeGameplayTag TAG_Data_Damage;
