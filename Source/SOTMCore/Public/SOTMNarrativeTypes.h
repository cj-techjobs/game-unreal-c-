// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SOTMNarrativeTypes.generated.h"

/** Ordinal is priority: higher value wins the interrupt, per the handbook's "critical transition/death -> boss line -> objective instruction -> manual hint -> ambient bark" queue order. */
UENUM(BlueprintType)
enum class EDialoguePriority : uint8
{
	AmbientBark,
	ManualHint,
	ObjectiveInstruction,
	BossLine,
	CriticalTransition
};

/** Ordinal is priority: the highest-priority currently-submitted intent wins ("one owner... explicit priorities"). */
UENUM(BlueprintType)
enum class EMusicIntent : uint8
{
	Menu,
	Exploration,
	Suspicion,
	Chase,
	BossPhase,
	Death
};

/**
 * One dialogue line request. Deliberately minimal: no voice asset,
 * subtitle text, language, or repeat/interruption policy fields, since
 * those belong to a dialogue-row data table (handbook Part 2 section 9.1)
 * that doesn't exist yet -- this covers the queue/priority mechanics, not
 * the content.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FDialogueCueRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Narrative")
	FGameplayTag CueId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Narrative")
	FGameplayTag SpeakerId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Narrative")
	EDialoguePriority Priority = EDialoguePriority::AmbientBark;
};
