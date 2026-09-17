// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMStation.h"
#include "Components/BoxComponent.h"
#include "ProgressionSubsystem.h"
#include "UpgradeDefinition.h"
#include "AbilityDefinition.h"
#include "Engine/GameInstance.h"

ASOTMStation::ASOTMStation()
{
	PrimaryActorTick.bCanEverTick = false;

	BodyVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyVolume"));
	BodyVolume->InitBoxExtent(FVector(40.0f, 40.0f, 55.0f));
	BodyVolume->SetCollisionProfileName(TEXT("BlockAll"));
	SetRootComponent(BodyVolume);
}

void ASOTMStation::BeginPlay()
{
	Super::BeginPlay();

	if (AbilityToRegister)
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr)
		{
			Progression->RegisterAbilityDefinition(AbilityToRegister);
		}
	}
}

FInteractionOffer ASOTMStation::GetInteractionOffer(const FInteractionRequest& Request) const
{
	FInteractionOffer Offer;
	Offer.bAvailable = (UpgradeToSell != nullptr);
	Offer.Operation = Request.Operation;
	Offer.Prompt = UpgradeToSell
		? FText::Format(NSLOCTEXT("SOTM", "PurchaseUpgradeFmt", "Purchase {0}"), UpgradeToSell->DisplayName)
		: NSLOCTEXT("SOTM", "PurchaseUnavailable", "Nothing to purchase");
	return Offer;
}

FCommandResult ASOTMStation::TryInteract(const FInteractionRequest& Request)
{
	FCommandResult Result;
	Result.OperationId = Request.RequestId;

	if (!UpgradeToSell)
	{
		return Result;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (!Progression)
	{
		return Result;
	}

	Result = Progression->TryPurchaseUpgrade(UpgradeToSell);
	Result.OperationId = Request.RequestId;
	return Result;
}
