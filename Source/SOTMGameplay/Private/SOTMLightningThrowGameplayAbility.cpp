// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMLightningThrowGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "SOTMLightningThrowCooldownGameplayEffect.h"
#include "SOTMLightningProjectile.h"
#include "SOTMStatusTags.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

USOTMLightningThrowGameplayAbility::USOTMLightningThrowGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	CooldownGameplayEffectClass = USOTMLightningThrowCooldownGameplayEffect::StaticClass();

	ActivationBlockedTags.AddTag(TAG_State_Stunned);
}

void USOTMLightningThrowGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	SpawnProjectile();

	UAbilityTask_PlayMontageAndWait* MontageTask = ThrowMontage
		? UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ThrowMontage)
		: nullptr;

	if (!MontageTask)
	{
		// No art assigned yet (e.g. a scaffolded PIE test) -- end cleanly
		// rather than hang waiting for a montage that will never play.
		HandleMontageFinished();
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &USOTMLightningThrowGameplayAbility::HandleMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &USOTMLightningThrowGameplayAbility::HandleMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &USOTMLightningThrowGameplayAbility::HandleMontageFinished);
	MontageTask->ReadyForActivation();
}

void USOTMLightningThrowGameplayAbility::SpawnProjectile()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	AActor* Avatar = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (!Avatar || !ProjectileClass)
	{
		return;
	}

	// Aim along the camera's look direction when a PlayerController/camera
	// is available (handbook Part 1 section 1: first-person exploration);
	// fall back to the avatar's own facing otherwise (e.g. no local player
	// camera yet in a scaffolded test).
	FRotator AimRotation = Avatar->GetActorRotation();
	if (ActorInfo->PlayerController.IsValid())
	{
		if (APlayerCameraManager* CameraManager = ActorInfo->PlayerController->PlayerCameraManager)
		{
			AimRotation = CameraManager->GetCameraRotation();
		}
	}

	const FVector SpawnLocation = Avatar->GetActorLocation() + Avatar->GetActorRotation().RotateVector(MuzzleOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Cast<APawn>(Avatar);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	GetWorld()->SpawnActor<ASOTMLightningProjectile>(ProjectileClass, SpawnLocation, AimRotation, SpawnParams);
}

void USOTMLightningThrowGameplayAbility::HandleMontageFinished()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), /*bReplicateEndAbility=*/true, /*bWasCancelled=*/false);
}
