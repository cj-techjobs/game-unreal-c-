// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMChest.h"
#include "Components/BoxComponent.h"
#include "SOTMPersistenceComponent.h"
#include "ProgressionSubsystem.h"
#include "Engine/GameInstance.h"

ASOTMChest::ASOTMChest()
{
	PrimaryActorTick.bCanEverTick = false;

	BodyVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyVolume"));
	BodyVolume->InitBoxExtent(FVector(50.0f, 35.0f, 40.0f));
	BodyVolume->SetCollisionProfileName(TEXT("BlockAll"));
	SetRootComponent(BodyVolume);

	PersistenceComponent = CreateDefaultSubobject<USOTMPersistenceComponent>(TEXT("PersistenceComponent"));
}

FInteractionOffer ASOTMChest::GetInteractionOffer(const FInteractionRequest& Request) const
{
	FInteractionOffer Offer;
	Offer.bAvailable = !bOpened;
	Offer.Operation = Request.Operation;
	Offer.Prompt = NSLOCTEXT("SOTM", "OpenChest", "Open Chest");
	return Offer;
}

FCommandResult ASOTMChest::TryInteract(const FInteractionRequest& Request)
{
	if (bOpened)
	{
		FCommandResult Result;
		Result.OperationId = Request.RequestId;
		return Result;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (!Progression)
	{
		FCommandResult Result;
		Result.OperationId = Request.RequestId;
		return Result;
	}

	FCommandResult Result = Progression->TryOpenChest(RequiredKeyCategory, RewardAmount);
	Result.OperationId = Request.RequestId;

	if (Result.bSucceeded)
	{
		bOpened = true;
	}

	return Result;
}

TArray<uint8> ASOTMChest::CapturePersistencePayload() const
{
	return { bOpened ? uint8(1) : uint8(0) };
}

void ASOTMChest::ApplyPersistencePayload(const TArray<uint8>& Payload)
{
	if (Payload.Num() > 0 && Payload[0] != 0)
	{
		bOpened = true;
	}
}
