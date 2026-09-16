// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "SOTMCinematicRoleProvider.generated.h"

UINTERFACE(MinimalAPI)
class USOTMCinematicRoleProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Resolves a requested cinematic role (e.g. "Camera.Jumpscare") to a
 * compatible live participant, so a Sequencer binding never hard-references
 * one specific placed actor.
 */
class SOTMCORE_API ISOTMCinematicRoleProvider
{
	GENERATED_BODY()

public:
	virtual AActor* ResolveCinematicRole(const FGameplayTag& Role) const = 0;
};
