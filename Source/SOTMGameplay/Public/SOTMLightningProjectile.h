// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMLightningProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 * Lightning Throw's projectile (handbook Part 2 section 3.1: "executes one
 * projectile or trace policy; applies stun to eligible enemies" --
 * projectile chosen as the authoritative collision path since the existing
 * presentation assets, e.g. P_LightningTrail, are a traveling-projectile
 * look, not a hitscan). Self-contained and self-cleaning: applies
 * USOTMStunGameplayEffect to the first ASC-bearing actor it overlaps (other
 * than its own instigator) and destroys itself, which trivially satisfies
 * "resolve each permitted target once" with no separate dedup bookkeeping.
 *
 * Collision uses hand-configured per-channel responses set directly in the
 * constructor (Block WorldStatic/WorldDynamic, Overlap Pawn, Ignore the
 * rest) rather than a named collision profile, so no project collision
 * settings/.ini need editing: "A visual beam is not evidence of a valid hit
 * through a wall" (handbook Part 2 section 3.1) -- it must actually stop at
 * solid geometry via OnComponentHit, not just fly through and rely on
 * overlap alone.
 *
 * Spawned directly by USOTMLightningThrowGameplayAbility::ActivateAbility
 * (no custom UAbilityTask) and owns its own full lifecycle from there,
 * matching the handbook's "Ability-specific tasks own their spawned
 * projectiles" ownership language even though the ability itself is not a
 * task -- it "fires and forgets" this actor, which is the "task" that owns
 * itself.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMLightningProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASOTMLightningProjectile();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Lightning")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Lightning")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** Spawned (if set) at whatever this projectile last touched -- a wall it blocked against, or an eligible target it stunned. Assign a Blueprint child of this class (e.g. BP_LightningBurst) since the actual VFX/SFX asset is content, not code. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Lightning")
	TSubclassOf<AActor> ImpactEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Lightning", meta = (ClampMin = "0"))
	float ProjectileSpeed = 2000.0f;

	/** Self-destructs with no impact effect once this far from its spawn point without hitting anything -- a fizzled miss, not an error. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Lightning", meta = (ClampMin = "0"))
	float MaxRangeCm = 3000.0f;

private:
	UFUNCTION()
	void HandleComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void HandleComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ResolveImpact(const FVector& ImpactLocation);

	FVector SpawnLocation = FVector::ZeroVector;
};
