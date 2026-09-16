// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SOTMGameInstance.generated.h"

/**
 * Application session bootstrap. Minimal by design for this increment:
 * service/subsystem wiring (UGameFlowSubsystem, USaveSubsystem,
 * UProgressionSubsystem) is added once the remaining M1 framework
 * classes and contracts exist.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USOTMGameInstance();
};
