// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMKeyPickup.h"
#include "Components/SphereComponent.h"
#include "SOTMPersistenceComponent.h"
#include "ProgressionSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"

ASOTMKeyPickup::ASOTMKeyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	CollectionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionVolume"));
	CollectionVolume->InitSphereRadius(75.0f);
	CollectionVolume->SetCollisionProfileName(TEXT("Trigger"));
	CollectionVolume->OnComponentBeginOverlap.AddDynamic(this, &ASOTMKeyPickup::HandleBeginOverlap);
	SetRootComponent(CollectionVolume);

	PersistenceComponent = CreateDefaultSubobject<USOTMPersistenceComponent>(TEXT("PersistenceComponent"));
}

void ASOTMKeyPickup::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bConsumed)
	{
		return;
	}

	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (!Progression)
	{
		return;
	}

	const FCommandResult Result = Progression->TryGrantKey(AccessCategory, Quantity);
	if (!Result.bSucceeded)
	{
		return;
	}

	MarkConsumedAndHide();
}

void ASOTMKeyPickup::MarkConsumedAndHide()
{
	bConsumed = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

TArray<uint8> ASOTMKeyPickup::CapturePersistencePayload() const
{
	return { bConsumed ? uint8(1) : uint8(0) };
}

void ASOTMKeyPickup::ApplyPersistencePayload(const TArray<uint8>& Payload)
{
	if (Payload.Num() > 0 && Payload[0] != 0)
	{
		MarkConsumedAndHide();
	}
}
