// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMGameplayAbility.h"
#include "SOTMGameplay.h"
#include "AbilitySystemComponent.h"

USOTMGameplayAbility::USOTMGameplayAbility()
{
}

bool USOTMGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayTagContainer LocalRelevantTags;
	FGameplayTagContainer* RelevantTagsToUse = OptionalRelevantTags ? OptionalRelevantTags : &LocalRelevantTags;

	const bool bCanActivate = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, RelevantTagsToUse);

	if (!bCanActivate)
	{
		UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: activation blocked (%s)"), *GetName(), *RelevantTagsToUse->ToStringSimple());
	}

	return bCanActivate;
}

void USOTMGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: activated on %s"), *GetName(), ActorInfo ? *GetNameSafe(ActorInfo->AvatarActor.Get()) : TEXT("<no avatar>"));

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void USOTMGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: ended (%s)"), *GetName(), bWasCancelled ? TEXT("cancelled") : TEXT("completed"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void USOTMGameplayAbility::RequestCancelSOTMAbility()
{
	if (!IsActive())
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return;
	}

	CancelAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), /*bReplicateCancelAbility=*/true);
}
