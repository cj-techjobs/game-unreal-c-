// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMSpeedBoostGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "SOTMSpeedBoostGameplayEffect.h"
#include "SOTMSpeedBoostCooldownGameplayEffect.h"
#include "SOTMMovementPolicyComponent.h"
#include "SOTMStatusTags.h"

USOTMSpeedBoostGameplayAbility::USOTMSpeedBoostGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = USOTMSpeedBoostCooldownGameplayEffect::StaticClass();

	ActivationBlockedTags.AddTag(TAG_State_Stunned);
	ActivationBlockedTags.AddTag(TAG_State_SpeedBoosted);
}

void USOTMSpeedBoostGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	BP_ApplyGameplayEffectToOwner(USOTMSpeedBoostGameplayEffect::StaticClass(), GetAbilityLevel());

	if (AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)
	{
		if (USOTMMovementPolicyComponent* MovementPolicy = Avatar->FindComponentByClass<USOTMMovementPolicyComponent>())
		{
			MovementPolicy->SetSpeedBoostMultiplier(SpeedMultiplier);
		}
	}

	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, BoostDurationSeconds);
	if (!WaitDelayTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
		return;
	}

	WaitDelayTask->OnFinish.AddDynamic(this, &USOTMSpeedBoostGameplayAbility::HandleBoostDurationElapsed);
	WaitDelayTask->ReadyForActivation();
}

void USOTMSpeedBoostGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Unconditional: covers normal completion AND cancel/interrupt (e.g.
	// stunned mid-boost) -- the ability owns clearing exactly the temporary
	// modifier it applied, nothing more (handbook Part 2 section 3.2:
	// "Cancellation removes only effects owned by the action being
	// cancelled").
	if (AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr)
	{
		if (USOTMMovementPolicyComponent* MovementPolicy = Avatar->FindComponentByClass<USOTMMovementPolicyComponent>())
		{
			MovementPolicy->SetSpeedBoostMultiplier(1.0f);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USOTMSpeedBoostGameplayAbility::HandleBoostDurationElapsed()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}
