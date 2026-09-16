// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SOTMPlayerController.generated.h"

/**
 * Local intent, possession, and interaction requests. No purchase or
 * death authority belongs in its input handlers.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASOTMPlayerController();
};
