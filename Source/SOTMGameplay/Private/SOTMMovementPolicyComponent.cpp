// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMMovementPolicyComponent.h"
#include "MovementDefinition.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USOTMMovementPolicyComponent::USOTMMovementPolicyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USOTMMovementPolicyComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());

	if (!MovementDefinition)
	{
		return;
	}

	CurrentStamina = MovementDefinition->MaxStamina;

	if (ACharacter* Character = OwningCharacter.Get())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->MaxAcceleration = MovementDefinition->Acceleration;
			Movement->BrakingDecelerationWalking = MovementDefinition->BrakingDeceleration;
			Movement->AirControl = MovementDefinition->AirControl;
			Movement->MaxWalkSpeedCrouched = MovementDefinition->CrouchSpeed;

			// FNavAgentProperties::bCanCrouch defaults to false on a fresh
			// CharacterMovementComponent -- ACharacter::Crouch() silently
			// no-ops via CanCrouchInCurrentState() until this is set,
			// independently of RequestStance()'s own bCanCrouch check below.
			Movement->GetNavAgentPropertiesRef().bCanCrouch = MovementDefinition->bCanCrouch;
		}
	}
}

void USOTMMovementPolicyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!MovementDefinition)
	{
		return;
	}

	const EMovementGait PreviousEffectiveGait = EffectiveGait;
	const bool bPreviousCrouched = OwningCharacter.IsValid() && OwningCharacter->bIsCrouched;

	EffectiveGait = ComputeEffectiveGait();
	ApplyStaminaForGait(EffectiveGait, DeltaTime);

	if (ACharacter* Character = OwningCharacter.Get())
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->MaxWalkSpeed = FMath::FInterpTo(Movement->MaxWalkSpeed, GetTargetSpeed(), DeltaTime, MovementDefinition->SpeedTransitionSpeed);
		}
	}

	const bool bCurrentCrouched = OwningCharacter.IsValid() && OwningCharacter->bIsCrouched;
	if (EffectiveGait != PreviousEffectiveGait || bCurrentCrouched != bPreviousCrouched)
	{
		OnMovementStateChanged.Broadcast(GetMovementSnapshot());
	}
}

void USOTMMovementPolicyComponent::RequestGait(EMovementGait Gait)
{
	RequestedGait = Gait;
}

void USOTMMovementPolicyComponent::RequestStance(bool bWantsCrouch)
{
	if (!MovementDefinition || !MovementDefinition->bCanCrouch)
	{
		return;
	}

	if (ACharacter* Character = OwningCharacter.Get())
	{
		if (bWantsCrouch)
		{
			Character->Crouch();
		}
		else
		{
			Character->UnCrouch();
		}
	}
}

void USOTMMovementPolicyComponent::SetMovementRestricted(bool bRestricted)
{
	bMovementRestricted = bRestricted;
}

bool USOTMMovementPolicyComponent::IsSprintToggleMode() const
{
	return MovementDefinition && MovementDefinition->bSprintIsToggle;
}

FMovementSnapshot USOTMMovementPolicyComponent::GetMovementSnapshot() const
{
	FMovementSnapshot Snapshot;
	Snapshot.RequestedGait = RequestedGait;
	Snapshot.EffectiveGait = EffectiveGait;
	Snapshot.bIsCrouched = OwningCharacter.IsValid() && OwningCharacter->bIsCrouched;
	Snapshot.bIsRestricted = bMovementRestricted;
	Snapshot.CurrentStamina = CurrentStamina;
	Snapshot.MaxStamina = MovementDefinition ? MovementDefinition->MaxStamina : 0.0f;
	Snapshot.EffectiveMaxSpeed = GetTargetSpeed();
	return Snapshot;
}

EMovementGait USOTMMovementPolicyComponent::ComputeEffectiveGait()
{
	if (bMovementRestricted || RequestedGait == EMovementGait::Walk)
	{
		bSprintLatchedOff = false;
		return EMovementGait::Walk;
	}

	const bool bCrouched = OwningCharacter.IsValid() && OwningCharacter->bIsCrouched;
	if (bCrouched)
	{
		return EMovementGait::Walk;
	}

	if (bSprintLatchedOff && CurrentStamina >= MovementDefinition->SprintResumeThreshold)
	{
		bSprintLatchedOff = false;
	}

	return bSprintLatchedOff ? EMovementGait::Walk : EMovementGait::Sprint;
}

void USOTMMovementPolicyComponent::ApplyStaminaForGait(EMovementGait Gait, float DeltaTime)
{
	if (Gait == EMovementGait::Sprint)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - MovementDefinition->StaminaDrainPerSecond * DeltaTime);
		TimeSinceSprintStopped = 0.0f;

		if (CurrentStamina <= MovementDefinition->ExhaustionThreshold)
		{
			bSprintLatchedOff = true;
		}
	}
	else
	{
		TimeSinceSprintStopped += DeltaTime;
		if (TimeSinceSprintStopped >= MovementDefinition->StaminaRecoveryDelay)
		{
			CurrentStamina = FMath::Min(MovementDefinition->MaxStamina, CurrentStamina + MovementDefinition->StaminaRecoveryPerSecond * DeltaTime);
		}
	}
}

float USOTMMovementPolicyComponent::GetTargetSpeed() const
{
	if (!MovementDefinition)
	{
		return 0.0f;
	}

	return EffectiveGait == EMovementGait::Sprint ? MovementDefinition->SprintSpeed : MovementDefinition->WalkSpeed;
}
