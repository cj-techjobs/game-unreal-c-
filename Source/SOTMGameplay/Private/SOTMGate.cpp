// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMGate.h"
#include "Components/BoxComponent.h"
#include "SOTMPersistenceComponent.h"
#include "ProgressionSubsystem.h"
#include "Engine/GameInstance.h"

ASOTMGate::ASOTMGate()
{
	PrimaryActorTick.bCanEverTick = false;

	BodyVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyVolume"));
	BodyVolume->InitBoxExtent(FVector(20.0f, 100.0f, 150.0f));
	BodyVolume->SetCollisionProfileName(TEXT("BlockAll"));
	SetRootComponent(BodyVolume);

	PersistenceComponent = CreateDefaultSubobject<USOTMPersistenceComponent>(TEXT("PersistenceComponent"));
}

FInteractionOffer ASOTMGate::GetInteractionOffer(const FInteractionRequest& Request) const
{
	FInteractionOffer Offer;
	Offer.bAvailable = !bOpen;
	Offer.Operation = Request.Operation;
	Offer.Prompt = NSLOCTEXT("SOTM", "OpenGate", "Open Gate");
	return Offer;
}

FCommandResult ASOTMGate::TryInteract(const FInteractionRequest& Request)
{
	FCommandResult Result;
	Result.OperationId = Request.RequestId;

	if (bOpen)
	{
		return Result;
	}

	if (RequiredKeyCategory.IsNone())
	{
		SetOpenState(true);
		Result.bSucceeded = true;
		return Result;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (!Progression)
	{
		return Result;
	}

	const FCommandResult KeyResult = Progression->TryConsumeKey(RequiredKeyCategory);
	if (!KeyResult.bSucceeded)
	{
		Result.FailureReason = KeyResult.FailureReason;
		return Result;
	}

	SetOpenState(true);
	Result.bSucceeded = true;
	Result.Revision = KeyResult.Revision;
	return Result;
}

void ASOTMGate::SetOpenState(bool bNewOpen)
{
	bOpen = bNewOpen;
	BodyVolume->SetCollisionEnabled(bOpen ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
}

TArray<uint8> ASOTMGate::CapturePersistencePayload() const
{
	return { bOpen ? uint8(1) : uint8(0) };
}

void ASOTMGate::ApplyPersistencePayload(const TArray<uint8>& Payload)
{
	if (Payload.Num() > 0 && Payload[0] != 0)
	{
		SetOpenState(true);
	}
}
