// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMCurrencyPickup.h"
#include "Components/SphereComponent.h"
#include "SOTMPersistenceComponent.h"
#include "ProgressionSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

ASOTMCurrencyPickup::ASOTMCurrencyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	CollectionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionVolume"));
	CollectionVolume->InitSphereRadius(75.0f);
	CollectionVolume->SetCollisionProfileName(TEXT("Trigger"));
	CollectionVolume->OnComponentBeginOverlap.AddDynamic(this, &ASOTMCurrencyPickup::HandleBeginOverlap);
	SetRootComponent(CollectionVolume);

	PersistenceComponent = CreateDefaultSubobject<USOTMPersistenceComponent>(TEXT("PersistenceComponent"));
}

void ASOTMCurrencyPickup::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

	const FCommandResult Result = Progression->TryCollectPickup(RewardAmount);
	if (!Result.bSucceeded)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(100, 3.0f, FColor::Yellow, FString::Printf(TEXT("+%lld currency (wallet: %lld)"), RewardAmount, Progression->GetSnapshot().Wallet));
	}

	MarkConsumedAndHide();
}

void ASOTMCurrencyPickup::MarkConsumedAndHide()
{
	bConsumed = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

TArray<uint8> ASOTMCurrencyPickup::CapturePersistencePayload() const
{
	return { bConsumed ? uint8(1) : uint8(0) };
}

void ASOTMCurrencyPickup::ApplyPersistencePayload(const TArray<uint8>& Payload)
{
	if (Payload.Num() > 0 && Payload[0] != 0)
	{
		MarkConsumedAndHide();
	}
}
