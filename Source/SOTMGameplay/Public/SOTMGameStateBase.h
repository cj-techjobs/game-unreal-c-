// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SOTMGameStateBase.generated.h"

/**
 * Observable current-world status (chapter/phase/readiness) as a
 * snapshot/projection. Not a second progression ledger.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASOTMGameStateBase();
};
