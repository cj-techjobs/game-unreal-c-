// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SOTMGameModeBase.generated.h"

/**
 * Base world-rules class. ASOTMFrontendGameMode and ASOTMChapterGameMode
 * are the concrete world configurations that will subclass this.
 */
UCLASS(Abstract)
class SOTMGAMEPLAY_API ASOTMGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASOTMGameModeBase();
};
