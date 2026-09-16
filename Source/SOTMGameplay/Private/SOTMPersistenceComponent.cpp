// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMPersistenceComponent.h"
#include "SOTMPersistable.h"
#include "WorldStateSubsystem.h"

USOTMPersistenceComponent::USOTMPersistenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USOTMPersistenceComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (!ParticipantId.IsValid())
	{
		ParticipantId = FGuid::NewGuid();
	}
}

void USOTMPersistenceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorldStateSubsystem* WorldState = GetWorld()->GetSubsystem<UWorldStateSubsystem>())
	{
		WorldState->RegisterParticipant(this);
	}
}

void USOTMPersistenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorldStateSubsystem* WorldState = GetWorld()->GetSubsystem<UWorldStateSubsystem>())
	{
		WorldState->UnregisterParticipant(this);
	}

	Super::EndPlay(EndPlayReason);
}

FWorldStateRecord USOTMPersistenceComponent::CaptureWorldStateRecord() const
{
	FWorldStateRecord Record;
	Record.ParticipantId = ParticipantId;

	if (ISOTMPersistable* Persistable = Cast<ISOTMPersistable>(GetOwner()))
	{
		Record.Payload = Persistable->CapturePersistencePayload();
	}

	return Record;
}

void USOTMPersistenceComponent::ApplyWorldStateRecord(const FWorldStateRecord& Record)
{
	if (ISOTMPersistable* Persistable = Cast<ISOTMPersistable>(GetOwner()))
	{
		Persistable->ApplyPersistencePayload(Record.Payload);
	}
}

void USOTMPersistenceComponent::ResetTransientState()
{
	if (ISOTMPersistable* Persistable = Cast<ISOTMPersistable>(GetOwner()))
	{
		Persistable->ResetPersistenceTransientState();
	}
}

bool USOTMPersistenceComponent::IsReadyForWorldState() const
{
	ISOTMPersistable* Persistable = Cast<ISOTMPersistable>(GetOwner());
	return Persistable && Persistable->IsReadyForPersistence();
}
