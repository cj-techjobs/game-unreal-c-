// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MovementDefinition.generated.h"

/**
 * Permitted locomotion settings for USOTMMovementPolicyComponent (handbook
 * Part 2 section 2 / Part 3's authoring surfaces table): walk/sprint/crouch
 * speed, acceleration/braking, air control, sprint stamina drain/recovery,
 * and hold-vs-toggle sprint input behavior.
 *
 * Not yet registered as an Asset Manager primary type: nothing in the
 * project picks a movement profile by runtime ID the way chapters are
 * chosen by chapter-select -- USOTMMovementPolicyComponent just holds a
 * direct EditDefaultsOnly reference. ContentId/GetPrimaryAssetId still
 * follow the same pattern as the other definition types so registering it
 * later, if something needs dynamic lookup, is a config-only change.
 *
 * Deliberately omits a generic "supported stance transitions" table and a
 * sprint-while-crouched toggle: crouch is the only non-standing stance that
 * exists, and USOTMMovementPolicyComponent doesn't support sprinting while
 * crouched in this pass, so bCanCrouch is the only stance fact worth a
 * field right now.
 */
UCLASS(BlueprintType)
class SOTMCORE_API UMovementDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName ContentId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0"))
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0"))
	float SprintSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0"))
	float CrouchSpeed = 200.0f;

	/** Maps to UCharacterMovementComponent::MaxAcceleration -- the sole source of speed-change ramp feel (see USOTMMovementPolicyComponent::TickComponent's comment for why MaxWalkSpeed itself is no longer separately eased). Tuned so the largest plausible jump (walk to a boosted sprint) still settles within ~0.5s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0"))
	float Acceleration = 2048.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0"))
	float BrakingDeceleration = 2048.0f;

	/** Maps directly to UCharacterMovementComponent::AirControl. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed", meta = (ClampMin = "0", ClampMax = "1"))
	float AirControl = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stance")
	bool bCanCrouch = true;

	/** If true, pressing the sprint action flips the requested gait; if false, sprint is held. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
	bool bSprintIsToggle = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float MaxStamina = 100.0f;

	/** 0 means sprint is never stamina-limited -- holding the sprint input keeps sprinting indefinitely. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float StaminaDrainPerSecond = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float StaminaRecoveryPerSecond = 15.0f;

	/** Seconds of not sprinting before recovery starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float StaminaRecoveryDelay = 1.5f;

	/** At/below this stamina, sprint force-stops (the "stamina exhausted" transition). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float ExhaustionThreshold = 0.0f;

	/** Stamina must recover to at least this level before an exhausted latch clears on its own, giving hysteresis instead of flickering at the exhaustion boundary. Should be >= ExhaustionThreshold. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (ClampMin = "0"))
	float SprintResumeThreshold = 20.0f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return ContentId.IsNone() ? FPrimaryAssetId()
			: FPrimaryAssetId(FPrimaryAssetType(TEXT("MovementProfile")), ContentId);
	}
};
