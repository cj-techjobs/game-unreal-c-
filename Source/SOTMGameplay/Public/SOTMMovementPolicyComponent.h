// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SOTMMovementTypes.h"
#include "SOTMMovementPolicyComponent.generated.h"

class ACharacter;
class UMovementDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementStateChanged, const FMovementSnapshot&, Snapshot);

/**
 * Resolves requested gait/stance into allowed CharacterMovement parameters
 * (handbook Part 2 section 2): the single writer of MaxWalkSpeed/
 * MaxWalkSpeedCrouched/MaxAcceleration/BrakingDecelerationWalking/
 * AirControl on the owning Character, so nothing else competes by
 * independently writing them and restoring stale cached values.
 *
 * Only usable on an ACharacter owner (it configures
 * UCharacterMovementComponent directly) -- Cast<ACharacter> here isn't the
 * kind of casting to avoid; ACharacter is the engine contract this
 * component is inherently about, and there is no lower-cost alternative to
 * reach its CharacterMovementComponent.
 *
 * Sprint-while-crouched isn't supported (Request sprint's guards include
 * "standing" per the transition table); crouch always forces Walk.
 */
UCLASS(ClassGroup = (SOTM), meta = (BlueprintSpawnableComponent))
class SOTMGAMEPLAY_API USOTMMovementPolicyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USOTMMovementPolicyComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Movement")
	FOnMovementStateChanged OnMovementStateChanged;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Movement")
	TObjectPtr<UMovementDefinition> MovementDefinition;

	/** Walk or Sprint; latched to Walk while stamina-exhausted, restricted, or crouched regardless of the request (see GetMovementSnapshot for the effective result). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Movement")
	void RequestGait(EMovementGait Gait);

	/** No-op if MovementDefinition disallows crouching. Uncrouch defers to ACharacter's own ceiling-clearance check -- it silently stays crouched if blocked. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Movement")
	void RequestStance(bool bWantsCrouch);

	/** Higher-priority gameplay restriction (stun/capture/death/cinematic). While true, effective gait is forced to Walk. Releasing re-evaluates current intent fresh rather than restoring a cached pre-restriction speed. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Movement")
	void SetMovementRestricted(bool bRestricted);

	UFUNCTION(BlueprintPure, Category = "SOTM|Movement")
	FMovementSnapshot GetMovementSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "SOTM|Movement")
	bool IsSprintToggleMode() const;

	/**
	 * Multiplies GetTargetSpeed()'s result; 1.0 = no boost. The sole hook a
	 * timed ability (USOTMSpeedBoostGameplayAbility) uses to affect movement
	 * speed, keeping this component the single writer of MaxWalkSpeed
	 * (handbook Part 2 section 2: "Abilities do not compete by independently
	 * writing MaxWalkSpeed"). The owning ability is responsible for resetting
	 * this to 1.0 in its own EndAbility on every terminal path, including
	 * cancel/interrupt.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Movement")
	void SetSpeedBoostMultiplier(float Multiplier);

private:
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwningCharacter;

	EMovementGait RequestedGait = EMovementGait::Walk;
	EMovementGait EffectiveGait = EMovementGait::Walk;
	bool bMovementRestricted = false;
	bool bSprintLatchedOff = false;
	float CurrentStamina = 0.0f;
	float TimeSinceSprintStopped = 0.0f;
	float SpeedBoostMultiplier = 1.0f;

	EMovementGait ComputeEffectiveGait();
	void ApplyStaminaForGait(EMovementGait Gait, float DeltaTime);
	float GetTargetSpeed() const;
};
