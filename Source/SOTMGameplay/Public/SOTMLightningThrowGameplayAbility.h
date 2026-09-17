// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMGameplayAbility.h"
#include "SOTMLightningThrowGameplayAbility.generated.h"

class ASOTMLightningProjectile;
class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;

/**
 * Lightning Throw (handbook Part 2 section 3.1): validates aim/cooldown,
 * spawns one ASOTMLightningProjectile toward the camera's aim direction,
 * and plays its throw montage for presentation. Ends when the montage
 * finishes/is interrupted/cancelled -- or immediately if no montage is
 * assigned, so a scaffolded test without art assigned doesn't hang.
 *
 * The projectile is self-contained (spawns, travels, resolves its own
 * hit/stun, destroys itself) -- this ability does not track or clean it up
 * itself, matching how a thrown object's flight outlives the throw motion.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMLightningThrowGameplayAbility : public USOTMGameplayAbility
{
	GENERATED_BODY()

public:
	USOTMLightningThrowGameplayAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SOTM|LightningThrow")
	TSubclassOf<ASOTMLightningProjectile> ProjectileClass;

	/** Presentation only -- see class comment for why activation doesn't wait on this to spawn the projectile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SOTM|LightningThrow")
	TObjectPtr<UAnimMontage> ThrowMontage;

	/** Forward/up offset from the avatar's location used as the projectile's spawn point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SOTM|LightningThrow")
	FVector MuzzleOffset = FVector(60.0f, 0.0f, 50.0f);

private:
	void SpawnProjectile();

	UFUNCTION()
	void HandleMontageFinished();
};
