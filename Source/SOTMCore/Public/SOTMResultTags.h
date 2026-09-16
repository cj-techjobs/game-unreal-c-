// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

/**
 * Shared FCommandResult::FailureReason vocabulary (handbook Part 2 section
 * 7.1's typed failure reasons: "Map reason codes to localized copy in
 * presentation. Do not use arbitrary strings for logic."). Native tags,
 * not DefaultGameplayTags.ini content: these are code constants every
 * command branches on, not designer-authored data, so they don't need the
 * AddTag/editor-registration path content tags use.
 *
 * Declared here (not with the bare UE_DECLARE_GAMEPLAY_TAG_EXTERN macro,
 * which expands to a plain "extern FNativeGameplayTag" with no DLL export
 * specifier) because these are consumed from other modules -- a plain
 * extern global would not resolve across the SOTMCore/SOTMGameplay module
 * boundary on Windows without SOTMCORE_API.
 */
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_InsufficientFunds;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_AlreadyOwned;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_MissingPrerequisite;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_StationUnavailable;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_ContentUnavailable;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_InvalidTarget;
extern SOTMCORE_API FNativeGameplayTag TAG_Result_Failure_Busy;
