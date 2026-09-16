// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMEnemyCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "SOTMAttributeSetHealth.h"
#include "SOTMDamageGameplayEffect.h"
#include "SOTMDamageTags.h"

ASOTMEnemyCharacter::ASOTMEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthAttributeSet = CreateDefaultSubobject<USOTMAttributeSetHealth>(TEXT("HealthAttributeSet"));
}

void ASOTMEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ASOTMEnemyCharacter::AttackPlayer()
{
	if (bIsAttacking)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !AttackMontage)
	{
		return;
	}

	bIsAttacking = true;

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ASOTMEnemyCharacter::OnAttackMontageEnded);
	AnimInstance->Montage_Play(AttackMontage);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
}

void ASOTMEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttacking = false;

	if (!bInterrupted)
	{
		ResolveAttackHit();
	}
}

void ASOTMEnemyCharacter::ResolveAttackHit()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * AttackTraceDistance;

	FHitResult Hit;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		this, Start, End, AttackTraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);

	if (!bHit || !Hit.GetActor())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Hit.GetActor());
	if (!TargetASC || !AbilitySystemComponent)
	{
		// Non-GAS receiver (handbook Part 1 section 4's ISOTMDamageReceiver
		// carve-out: "only for non-GAS environmental receivers") -- keep the
		// legacy TakeDamage path rather than forcing every hittable actor to
		// grow an ASC just to be damageable.
		UGameplayStatics::ApplyDamage(Hit.GetActor(), AttackDamage, GetController(), this, nullptr);
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(GetController(), this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(USOTMDamageGameplayEffect::StaticClass(), 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_Data_Damage, -AttackDamage);
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
