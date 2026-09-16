// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SOTMPlayerCameraManager.generated.h"

/**
 * Effective camera mode, blends, clamps, and accessibility scaling.
 * No story outcomes or life accounting.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ASOTMPlayerCameraManager();
};
