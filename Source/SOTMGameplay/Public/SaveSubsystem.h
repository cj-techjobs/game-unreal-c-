// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SOTMSaveGame.h"
#include "SaveSubsystem.generated.h"

/**
 * Save envelope IO: two alternating generation slots with revision and
 * checksum integrity, per the architecture handbook's Part 1 section 8.3
 * ("write the next generation without destroying the previous valid one
 * ... select the newest valid generation on load"). Synchronous IO only
 * for this increment -- no async queue/coalescing, no migrations, no
 * demo/full slot namespacing, no restoration-sequence orchestration.
 * Those depend on systems (GameFlow, Chapter, Progression) that don't
 * have real behavior yet; this establishes a real, testable save/load
 * round trip for them to build on.
 */
UCLASS()
class SOTMGAMEPLAY_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Writes a new envelope to whichever generation slot was written least recently, bumping SnapshotRevision. Returns false if the engine failed to write the slot. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Save")
	bool SaveSnapshot(const FProgressionSnapshot& Progression, const TArray<FWorldStateRecord>& RetainedWorldRecords, const TArray<FGuid>& ConsumedPickupIds);

	/** Loads and checksum-validates both generation slots, returning the newest valid one (or nullptr if neither validates). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Save")
	USOTMSaveGame* LoadLatestValid() const;

	UFUNCTION(BlueprintCallable, Category = "SOTM|Save")
	bool HasAnyValidSave() const;

private:
	static FString GetSlotNameForGeneration(int32 Generation);
	static USOTMSaveGame* LoadAndValidateSlot(const FString& SlotName);
};
