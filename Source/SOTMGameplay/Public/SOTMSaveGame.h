// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SOTMProgressionTypes.h"
#include "SOTMWorldStateTypes.h"
#include "SOTMSaveGame.generated.h"

/**
 * Save envelope header. Fields match the architecture handbook's Part 1
 * section 8.2 save-envelope shape. Checksum is computed over the rest of
 * the serialized envelope with this field zeroed, so it must be the last
 * field set before writing and the first field cleared before validating.
 */
USTRUCT(BlueprintType)
struct SOTMGAMEPLAY_API FSOTMSaveHeader
{
	GENERATED_BODY()

	static constexpr int32 ExpectedMagic = 0x53544D31; // 'STM1'

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int32 Magic = ExpectedMagic;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int32 SaveSchemaVersion = 1;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int32 ContentCatalogVersion = 1;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	FString BuildProfileId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int32 SlotGeneration = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int64 SnapshotRevision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	int32 Checksum = 0;
};

/**
 * Custom USaveGame envelope of plain records, per the handbook's Part 1
 * section 8.2 ("not a serialized actor graph"). Deliberately omits
 * Profile facts, UpgradeRanks/Inventory/ObjectiveRecords/AppliedOutcomeIds,
 * and Recovery fields -- those belong to systems (GameFlow, Progression,
 * Objective) that don't have real record shapes yet. Follows
 * FProgressionSnapshot's own precedent of not guessing a shape before its
 * owning system exists.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	FSOTMSaveHeader Header;

	/** Run scope: RunId, Revision, Wallet, RemainingLives (ChapterId/RunStatus/CheckpointDescriptor pending ChapterSubsystem). */
	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	FProgressionSnapshot Progression;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	TArray<FWorldStateRecord> RetainedWorldRecords;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Save")
	TArray<FGuid> ConsumedPickupIds;
};
