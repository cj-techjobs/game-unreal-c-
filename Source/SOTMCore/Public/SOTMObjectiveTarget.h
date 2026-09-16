// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SOTMObjectiveTarget.generated.h"

UINTERFACE(MinimalAPI)
class USOTMObjectiveTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Exposes a semantic marker/condition identity so an objective can target
 * "the forest entrance" without coupling to a specific actor class.
 */
class SOTMCORE_API ISOTMObjectiveTarget
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetObjectiveMarkerId() const = 0;
};
