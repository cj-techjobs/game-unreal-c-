// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMFrontendGameMode.h"
#include "GameFlowSubsystem.h"
#include "Engine/GameInstance.h"

ASOTMFrontendGameMode::ASOTMFrontendGameMode()
{
	// Intentionally empty for this increment.
}

void ASOTMFrontendGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UGameFlowSubsystem* GameFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameFlowSubsystem>() : nullptr)
	{
		GameFlow->EnterFrontend();
	}
}
