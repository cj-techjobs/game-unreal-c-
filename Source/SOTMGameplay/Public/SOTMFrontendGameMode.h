// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMGameModeBase.h"
#include "SOTMFrontendGameMode.generated.h"

/** Menu world configuration. Does not start chapter AI/encounters. */
UCLASS()
class SOTMGAMEPLAY_API ASOTMFrontendGameMode : public ASOTMGameModeBase
{
	GENERATED_BODY()

public:
	ASOTMFrontendGameMode();

	/** Drives UGameFlowSubsystem into Frontend -- nothing else in the project calls EnterFrontend() yet, so without this the flow state permanently sits at its Boot default. */
	virtual void BeginPlay() override;
};
