// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SOTMAttributeSetHealth.generated.h"

#define SOTM_GAMEPLAY_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSOTMHealthDepleted, AActor* /*OwnerActor*/);

/**
 * Health/MaxHealth (handbook Part 2 section 3: "Use GAS for Health...").
 * Stamina intentionally stays in USOTMMovementPolicyComponent's existing
 * plain-float implementation for this increment rather than moving here:
 * that system is already built and PIE-verified, and section 2's own
 * "Recommended Chapter 1 starting policy" explicitly allows normal
 * locomotion to keep "optional stamina" outside GAS while only the timed
 * Speed Boost modifier is GAS-driven -- migrating the working system now
 * would be a large, disruptive rework for no near-term gameplay need.
 *
 * OnHealthDepleted broadcasts once (edge-triggered) when Health crosses to
 * zero. Nothing currently subscribes to it -- the handbook's queued
 * terminal-candidate/GameFlow resolution phase (section 3.2) doesn't exist
 * yet and needs boss-design approval before it's built, so this stays a
 * fact broadcast, not a commit path. Do not wire this directly to
 * UProgressionSubsystem::TryAcceptDeath from here: that would decide a
 * death inside a GameplayEffect execution callback, exactly what section
 * 3.2 says not to do ("Queue terminal candidates rather than committing
 * deaths/victories inside hit callbacks").
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMAttributeSetHealth : public UAttributeSet
{
	GENERATED_BODY()

public:
	USOTMAttributeSetHealth();

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	SOTM_GAMEPLAY_ATTRIBUTE_ACCESSORS(USOTMAttributeSetHealth, Health)

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	SOTM_GAMEPLAY_ATTRIBUTE_ACCESSORS(USOTMAttributeSetHealth, MaxHealth)

	/** Fires once when Health crosses from >0 to <=0. */
	FOnSOTMHealthDepleted OnHealthDepleted;

	/** GAMEPLAYATTRIBUTE_VALUE_GETTER's GetHealth()/GetMaxHealth() aren't UFUNCTIONs (the macro predates BlueprintPure exposure); these thin wrappers are so presentation code (health bars, this task's own PIE verification) can read current values without a bespoke getter per caller. */
	UFUNCTION(BlueprintPure, Category = "SOTM|Attributes", DisplayName = "Get Health")
	float K2_GetHealth() const { return GetHealth(); }

	UFUNCTION(BlueprintPure, Category = "SOTM|Attributes", DisplayName = "Get Max Health")
	float K2_GetMaxHealth() const { return GetMaxHealth(); }

	/**
	 * Restores Health to MaxHealth and re-arms edge-triggered depletion
	 * detection for the next fatal hit. A plain SetHealth from outside would
	 * leave bWasAboveZero stuck false from the death that just happened, so
	 * a second fatal hit on the respawned avatar would never re-cross the
	 * >0-to-<=0 edge this class watches for and OnHealthDepleted would
	 * silently never fire again. Called by
	 * ASOTMPlayerCharacter::InitializeAbilityActorInfo on every (re)possession,
	 * respawn included.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Attributes")
	void ResetForRespawn();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

private:
	bool bWasAboveZero = true;
};
