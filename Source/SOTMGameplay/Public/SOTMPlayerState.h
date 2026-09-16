// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "SOTMPlayerState.generated.h"

class UAbilitySystemComponent;
class USOTMAttributeSetHealth;

/**
 * Player ASC/attribute sets across pawn replacement in the same world
 * (handbook Part 1 section 4's ASOTMPlayerState row). Not the disk save;
 * ordinary map travel may replace it.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASOTMPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Abilities")
	USOTMAttributeSetHealth* GetHealthAttributeSet() const { return HealthAttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Abilities")
	TObjectPtr<USOTMAttributeSetHealth> HealthAttributeSet;
};
