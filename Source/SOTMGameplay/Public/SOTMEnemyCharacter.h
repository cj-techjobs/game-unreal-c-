// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "SOTMEnemyCharacter.generated.h"

class UAnimMontage;
class UAbilitySystemComponent;
class USOTMAttributeSetHealth;

/**
 * Enemy body: ASC/attributes, movement, and animation. Does not decide
 * objective or chapter completion directly.
 *
 * Unlike ASOTMPlayerCharacter, this class owns its ASC directly rather than
 * forwarding to a PlayerState -- AI-controlled pawns have no PlayerState in
 * this project, and the handbook's own Part 1 section 4 row lists "ASC/
 * attributes" as belonging to the enemy body itself. Reuses
 * USOTMAttributeSetHealth (Health/MaxHealth semantics are identical to the
 * player's) rather than a second near-duplicate attribute set class.
 * Replication mode is Minimal, not the player's Mixed: Epic's own GAS
 * guidance recommends Minimal for AI/NPC-owned ASCs where remote clients
 * only need gameplay cues, not full effect bookkeeping.
 *
 * Damage dealt BY this enemy (AttackPlayer, ported from the original
 * CruelDoll Blueprint BP_AI) now applies USOTMDamageGameplayEffect to the
 * hit actor's ASC when it has one (player or another GAS actor), falling
 * back to the legacy UGameplayStatics::ApplyDamage/TakeDamage path only for
 * non-GAS receivers (handbook Part 1 section 4's ISOTMDamageReceiver
 * carve-out). This covers the pipeline's "effect calculation -> health
 * change" steps (handbook Part 2 section 3.2); "lethal candidate -> GameFlow
 * acceptance" is intentionally not built here -- see
 * USOTMAttributeSetHealth::OnHealthDepleted's own comment for why that needs
 * a queued/GameFlow-phase design the handbook explicitly flags as needing
 * boss-design approval, not something to improvise in this pass.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMEnemyCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASOTMEnemyCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Abilities")
	USOTMAttributeSetHealth* GetHealthAttributeSet() const { return HealthAttributeSet; }

	/** Plays the attack montage, then applies damage via a forward sphere trace when it completes. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Enemy")
	void AttackPlayer();

protected:
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Abilities")
	TObjectPtr<USOTMAttributeSetHealth> HealthAttributeSet;

	/** Montage played by AttackPlayer. Assign the existing AS_CruelDoll_Attack03_Montage on the Blueprint child. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** Damage applied to whatever the attack trace hits. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Attack")
	float AttackDamage = 10.0f;

	/** Distance, forward from the actor, that the attack hit trace sweeps. Kept comfortably beyond BTTask_SOTMChasePlayer's AcceptanceRadius so a completed chase is always within swing reach. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Attack")
	float AttackTraceDistance = 120.0f;

	/** Radius of the attack hit trace's sphere sweep. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Attack")
	float AttackTraceRadius = 25.0f;

private:
	bool bIsAttacking = false;

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void ResolveAttackHit();
};
