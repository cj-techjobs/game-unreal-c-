// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMIdentityTypes.h"
#include "SOTMProgressionTypes.generated.h"

/** Coarse run lifecycle. GameOver/Completed are terminal -- no further death/wallet commands apply once set. */
UENUM(BlueprintType)
enum class ERunStatus : uint8
{
	InProgress,
	GameOver,
	Completed
};

/**
 * Read-only snapshot of what the current run currently has.
 * ObjectiveRecords are still deferred -- no record shape exists yet -- but
 * OwnedUpgradeRanks is real (UUpgradeDefinition exists to key it by), and
 * KeyCounts is now real too: handbook Part 2 section 6.1's minimal
 * inventory start ("typed item definitions and integer quantities, not a
 * universal RPG inventory") for keys specifically, an access-category name
 * to an integer count. Notes/lore ("retained discoveries") still have no
 * definition shape, so they stay deferred alongside ObjectiveRecords.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FProgressionSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	FRunId RunId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	int64 Revision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	int64 Wallet = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	int32 RemainingLives = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	ERunStatus RunStatus = ERunStatus::InProgress;

	/** Keyed by UUpgradeDefinition::ContentId. Absent entries are rank 0 (not owned). */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	TMap<FName, int32> OwnedUpgradeRanks;

	/** Keyed by access-category name (e.g. "Key.Bronze"). Absent entries are count 0. */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Progression")
	TMap<FName, int32> KeyCounts;
};
