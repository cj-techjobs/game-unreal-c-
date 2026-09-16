// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "SOTMGameplayAbility.generated.h"

/**
 * Small native ability base (handbook Part 2 section 3: "Implement a small
 * native base ability with common cancellation, diagnostics, ownership, and
 * activation-result handling; designers can create approved Blueprint
 * ability subclasses and tune definitions.").
 *
 * Adds a diagnostic log around activation/failure/end so every ability --
 * native or a designer's Blueprint child -- gets consistent visibility
 * without each subclass wiring its own logging. Does not reimplement GAS's
 * own K2_ActivateAbility routing: Super::ActivateAbility/EndAbility are
 * still called so Blueprint ability children compiled from this class keep
 * working exactly as the Ability Blueprint editor already generates them.
 */
UCLASS(Abstract)
class SOTMGAMEPLAY_API USOTMGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	USOTMGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	/** Ends this ability as a clean cancel with sensible default params, so designers don't need to hunt for CancelAbility's full overload. No-ops if the ability isn't currently active. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Ability")
	void RequestCancelSOTMAbility();
};
