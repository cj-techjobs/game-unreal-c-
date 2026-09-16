// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Crc.h"
#include "Misc/App.h"

namespace
{
	uint32 ComputeChecksum(USOTMSaveGame* SaveGameObject)
	{
		const int32 StoredChecksum = SaveGameObject->Header.Checksum;
		SaveGameObject->Header.Checksum = 0;

		TArray<uint8> Bytes;
		UGameplayStatics::SaveGameToMemory(SaveGameObject, Bytes);
		const uint32 Computed = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());

		SaveGameObject->Header.Checksum = StoredChecksum;
		return Computed;
	}
}

FString USaveSubsystem::GetSlotNameForGeneration(int32 Generation)
{
	return Generation == 0 ? TEXT("SOTMSave_GenA") : TEXT("SOTMSave_GenB");
}

USOTMSaveGame* USaveSubsystem::LoadAndValidateSlot(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return nullptr;
	}

	USOTMSaveGame* Loaded = Cast<USOTMSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Loaded || Loaded->Header.Magic != FSOTMSaveHeader::ExpectedMagic)
	{
		return nullptr;
	}

	return ComputeChecksum(Loaded) == static_cast<uint32>(Loaded->Header.Checksum) ? Loaded : nullptr;
}

bool USaveSubsystem::SaveSnapshot(const FProgressionSnapshot& Progression, const TArray<FWorldStateRecord>& RetainedWorldRecords, const TArray<FGuid>& ConsumedPickupIds)
{
	USOTMSaveGame* ExistingA = LoadAndValidateSlot(GetSlotNameForGeneration(0));
	USOTMSaveGame* ExistingB = LoadAndValidateSlot(GetSlotNameForGeneration(1));

	const int64 RevisionA = ExistingA ? ExistingA->Header.SnapshotRevision : -1;
	const int64 RevisionB = ExistingB ? ExistingB->Header.SnapshotRevision : -1;

	// Write over whichever slot is older (or empty), keeping the other generation intact.
	const int32 GenerationToWrite = (RevisionA <= RevisionB) ? 0 : 1;
	const int64 NextRevision = FMath::Max(RevisionA, RevisionB) + 1;

	USOTMSaveGame* SaveGameObject = Cast<USOTMSaveGame>(UGameplayStatics::CreateSaveGameObject(USOTMSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		return false;
	}

	SaveGameObject->Progression = Progression;
	SaveGameObject->RetainedWorldRecords = RetainedWorldRecords;
	SaveGameObject->ConsumedPickupIds = ConsumedPickupIds;
	SaveGameObject->Header.BuildProfileId = FApp::GetBuildVersion();
	SaveGameObject->Header.SlotGeneration = GenerationToWrite;
	SaveGameObject->Header.SnapshotRevision = NextRevision;
	SaveGameObject->Header.Timestamp = FDateTime::UtcNow();
	SaveGameObject->Header.Checksum = static_cast<int32>(ComputeChecksum(SaveGameObject));

	return UGameplayStatics::SaveGameToSlot(SaveGameObject, GetSlotNameForGeneration(GenerationToWrite), 0);
}

USOTMSaveGame* USaveSubsystem::LoadLatestValid() const
{
	USOTMSaveGame* SlotA = LoadAndValidateSlot(GetSlotNameForGeneration(0));
	USOTMSaveGame* SlotB = LoadAndValidateSlot(GetSlotNameForGeneration(1));

	if (SlotA && SlotB)
	{
		return SlotA->Header.SnapshotRevision >= SlotB->Header.SnapshotRevision ? SlotA : SlotB;
	}
	return SlotA ? SlotA : SlotB;
}

bool USaveSubsystem::HasAnyValidSave() const
{
	return LoadLatestValid() != nullptr;
}
