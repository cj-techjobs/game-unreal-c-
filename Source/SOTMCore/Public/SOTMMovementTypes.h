// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMMovementTypes.generated.h"

/** Handbook Part 2 section 2's "requested gait" / "effective gait" dimensions. */
UENUM(BlueprintType)
enum class EMovementGait : uint8
{
	Walk,
	Sprint
};

/**
 * Read-only projection of USOTMMovementPolicyComponent's current state, for
 * animation/camera/HUD consumers (handbook Part 2 section 2's
 * GetMovementSnapshot). Physical movement mode isn't included -- that's
 * read directly from UCharacterMovementComponent, which already owns it.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FMovementSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	EMovementGait RequestedGait = EMovementGait::Walk;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	EMovementGait EffectiveGait = EMovementGait::Walk;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	bool bIsCrouched = false;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	bool bIsRestricted = false;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	float CurrentStamina = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	float MaxStamina = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Movement")
	float EffectiveMaxSpeed = 0.0f;
};
