// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SOTMObjectiveTypes.generated.h"

UENUM(BlueprintType)
enum class EObjectiveStatus : uint8
{
	Inactive,
	Active,
	Completed,
	Failed
};

/**
 * Runtime state for one objective, keyed by its ObjectiveId tag elsewhere.
 * Deliberately minimal: no prerequisites/rewards/marker-hint fields, since
 * those belong to UObjectiveDefinition (handbook Part 2 section 8), which
 * doesn't exist as a data asset type yet.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FObjectiveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Objective")
	FGameplayTag ObjectiveId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Objective")
	EObjectiveStatus Status = EObjectiveStatus::Inactive;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Objective")
	int32 Progress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Objective")
	int32 Target = 1;
};
