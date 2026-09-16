// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SOTMCompanionCharacter.generated.h"

/**
 * Optional physical guide representation. Critical objectives must
 * keep working if this actor is absent.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMCompanionCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASOTMCompanionCharacter();
};
