// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldStateSubsystem.h"

bool UWorldStateSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UWorldStateSubsystem::RegisterParticipant(UObject* Participant)
{
	ISOTMWorldStateParticipant* ParticipantInterface = Cast<ISOTMWorldStateParticipant>(Participant);
	if (!ParticipantInterface)
	{
		return;
	}

	ParticipantObjects.AddUnique(Participant);

	const FGuid ParticipantId = ParticipantInterface->GetParticipantId();
	if (const FWorldStateRecord* PendingRecord = PendingRecordsByParticipantId.Find(ParticipantId))
	{
		ParticipantInterface->ApplyWorldStateRecord(*PendingRecord);
		PendingRecordsByParticipantId.Remove(ParticipantId);
	}
}

void UWorldStateSubsystem::UnregisterParticipant(UObject* Participant)
{
	ParticipantObjects.Remove(Participant);
}

TArray<FWorldStateRecord> UWorldStateSubsystem::CaptureAllRecords() const
{
	TArray<FWorldStateRecord> Records;
	Records.Reserve(ParticipantObjects.Num());

	for (const TWeakObjectPtr<UObject>& WeakParticipant : ParticipantObjects)
	{
		if (ISOTMWorldStateParticipant* Participant = Cast<ISOTMWorldStateParticipant>(WeakParticipant.Get()))
		{
			if (Participant->IsReadyForWorldState())
			{
				Records.Add(Participant->CaptureWorldStateRecord());
			}
		}
	}
	return Records;
}

void UWorldStateSubsystem::ApplyRetainedRecords(const TArray<FWorldStateRecord>& Records)
{
	for (const FWorldStateRecord& Record : Records)
	{
		bool bApplied = false;
		for (const TWeakObjectPtr<UObject>& WeakParticipant : ParticipantObjects)
		{
			if (ISOTMWorldStateParticipant* Participant = Cast<ISOTMWorldStateParticipant>(WeakParticipant.Get()))
			{
				if (Participant->GetParticipantId() == Record.ParticipantId)
				{
					Participant->ApplyWorldStateRecord(Record);
					bApplied = true;
					break;
				}
			}
		}

		if (!bApplied)
		{
			PendingRecordsByParticipantId.Add(Record.ParticipantId, Record);
		}
	}
}

void UWorldStateSubsystem::ResetAllTransientState()
{
	for (const TWeakObjectPtr<UObject>& WeakParticipant : ParticipantObjects)
	{
		if (ISOTMWorldStateParticipant* Participant = Cast<ISOTMWorldStateParticipant>(WeakParticipant.Get()))
		{
			Participant->ResetTransientState();
		}
	}
}
