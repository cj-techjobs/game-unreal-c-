// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMGameplayAbility.h"
#include "SOTMSpeedBoostGameplayAbility.generated.h"

class UAbilityTask_WaitDelay;

/**
 * Speed Boost (handbook Part 2 section 3.1): a timed movement-speed
 * multiplier with a cooldown. Requires owned rank/grant (via the existing
 * UProgressionSubsystem::ReconcileAbilityGrants registry) and permitted
 * action state (blocked while Stunned or already boosted).
 *
 * Deliberately does not write CharacterMovementComponent::MaxWalkSpeed
 * itself -- USOTMMovementPolicyComponent stays the single writer (handbook
 * Part 2 section 2). This ability only calls its SetSpeedBoostMultiplier
 * hook and applies USOTMSpeedBoostGameplayEffect for the observable
 * State.SpeedBoosted tag/anti-restack guard; the two are independent and
 * both cleaned up unconditionally in EndAbility.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMSpeedBoostGameplayAbility : public USOTMGameplayAbility
{
	GENERATED_BODY()

public:
	USOTMSpeedBoostGameplayAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Multiplies the avatar's effective movement speed for the duration. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SOTM|SpeedBoost", meta = (ClampMin = "1.0"))
	float SpeedMultiplier = 1.5f;

	/** How long the boost (and this ability's own activation) lasts. Author USOTMSpeedBoostGameplayEffect's own duration to match if it's ever retuned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SOTM|SpeedBoost", meta = (ClampMin = "0.1"))
	float BoostDurationSeconds = 5.0f;

private:
	UFUNCTION()
	void HandleBoostDurationElapsed();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> WaitDelayTask;
};
