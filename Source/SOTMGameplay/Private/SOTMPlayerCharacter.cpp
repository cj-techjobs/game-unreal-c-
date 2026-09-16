// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMPlayerCharacter.h"
#include "Math/RotationMatrix.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "SOTMInteractionComponent.h"
#include "SOTMMovementPolicyComponent.h"
#include "SOTMMovementTypes.h"
#include "AbilitySystemComponent.h"
#include "SOTMPlayerState.h"
#include "SOTMAttributeSetHealth.h"
#include "ProgressionSubsystem.h"
#include "SOTMProgressionTypes.h"
#include "GameFlowSubsystem.h"
#include "Perception/AISense_Hearing.h"

ASOTMPlayerCharacter::ASOTMPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionComponent = CreateDefaultSubobject<USOTMInteractionComponent>(TEXT("InteractionComponent"));
	MovementPolicyComponent = CreateDefaultSubobject<USOTMMovementPolicyComponent>(TEXT("MovementPolicyComponent"));
}

void ASOTMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	FollowCameraRef = FindComponentByClass<UCameraComponent>();

	if (FollowCameraRef)
	{
		BaseFieldOfView = FollowCameraRef->FieldOfView;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (DefaultMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				FModifyContextOptions Options;
				Options.bIgnoreAllPressedKeysUntilRelease = true;
				Subsystem->AddMappingContext(DefaultMappingContext, 0, Options);
			}
		}
	}

	if (UProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<UProgressionSubsystem>() : nullptr)
	{
		Progression->OnProgressionChanged.AddDynamic(this, &ASOTMPlayerCharacter::HandleProgressionChanged);
	}
}

void ASOTMPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<UProgressionSubsystem>() : nullptr)
	{
		Progression->OnProgressionChanged.RemoveDynamic(this, &ASOTMPlayerCharacter::HandleProgressionChanged);
	}

	// HealthAttributeSet outlives this avatar (it's PlayerState-owned) -- unbind explicitly so a long session with many respawns doesn't accumulate stale bindings on it.
	if (ASOTMPlayerState* PS = GetPlayerState<ASOTMPlayerState>())
	{
		if (USOTMAttributeSetHealth* HealthSet = PS->GetHealthAttributeSet())
		{
			HealthSet->OnHealthDepleted.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ASOTMPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bEffectivelySprinting = MovementPolicyComponent
		&& MovementPolicyComponent->GetMovementSnapshot().EffectiveGait == EMovementGait::Sprint;
	const float TargetFOV = bEffectivelySprinting ? SprintFieldOfView : BaseFieldOfView;

	if (FollowCameraRef)
	{
		FollowCameraRef->FieldOfView = FMath::FInterpTo(FollowCameraRef->FieldOfView, TargetFOV, DeltaSeconds, SprintTransitionSpeed);
	}

	if (bEffectivelySprinting)
	{
		TimeSinceLastSprintNoise += DeltaSeconds;
		if (TimeSinceLastSprintNoise >= SprintNoiseInterval)
		{
			TimeSinceLastSprintNoise = 0.0f;
			UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), SprintNoiseLoudness, this, SprintNoiseMaxRange);
		}
	}
	else
	{
		// Next sprint start reports immediately rather than waiting a full interval.
		TimeSinceLastSprintNoise = SprintNoiseInterval;
	}
}

void ASOTMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASOTMPlayerCharacter::HandleMove);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASOTMPlayerCharacter::HandleLook);
		}
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASOTMPlayerCharacter::HandleSprintPressed);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASOTMPlayerCharacter::HandleSprintReleased);
		}
		if (CrouchAction)
		{
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASOTMPlayerCharacter::HandleCrouchPressed);
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ASOTMPlayerCharacter::HandleCrouchReleased);
		}
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASOTMPlayerCharacter::HandleInteract);
		}
	}
}

void ASOTMPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotationMatrix ControlRotMatrix(GetControlRotation());

	AddMovementInput(ControlRotMatrix.GetUnitAxis(EAxis::Y), MoveValue.X);
	AddMovementInput(ControlRotMatrix.GetUnitAxis(EAxis::X), MoveValue.Y);
}

void ASOTMPlayerCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

void ASOTMPlayerCharacter::HandleSprintPressed()
{
	if (!MovementPolicyComponent)
	{
		return;
	}

	if (MovementPolicyComponent->IsSprintToggleMode())
	{
		const bool bCurrentlyRequestingSprint = MovementPolicyComponent->GetMovementSnapshot().RequestedGait == EMovementGait::Sprint;
		MovementPolicyComponent->RequestGait(bCurrentlyRequestingSprint ? EMovementGait::Walk : EMovementGait::Sprint);
	}
	else
	{
		MovementPolicyComponent->RequestGait(EMovementGait::Sprint);
	}
}

void ASOTMPlayerCharacter::HandleSprintReleased()
{
	if (MovementPolicyComponent && !MovementPolicyComponent->IsSprintToggleMode())
	{
		MovementPolicyComponent->RequestGait(EMovementGait::Walk);
	}
}

void ASOTMPlayerCharacter::HandleCrouchPressed()
{
	if (MovementPolicyComponent)
	{
		MovementPolicyComponent->RequestStance(true);
	}
}

void ASOTMPlayerCharacter::HandleCrouchReleased()
{
	if (MovementPolicyComponent)
	{
		MovementPolicyComponent->RequestStance(false);
	}
}

void ASOTMPlayerCharacter::HandleInteract()
{
	if (InteractionComponent)
	{
		InteractionComponent->TryInteractWithCurrentTarget();
	}
}

UAbilitySystemComponent* ASOTMPlayerCharacter::GetAbilitySystemComponent() const
{
	const ASOTMPlayerState* PS = GetPlayerState<ASOTMPlayerState>();
	return PS ? PS->GetAbilitySystemComponent() : nullptr;
}

void ASOTMPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilityActorInfo();
}

void ASOTMPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitializeAbilityActorInfo();
}

void ASOTMPlayerCharacter::InitializeAbilityActorInfo()
{
	ASOTMPlayerState* PS = GetPlayerState<ASOTMPlayerState>();
	if (!PS)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	ASC->InitAbilityActorInfo(PS, this);

	if (UProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<UProgressionSubsystem>() : nullptr)
	{
		Progression->ReconcileAbilityGrants(ASC);
	}

	if (USOTMAttributeSetHealth* HealthSet = PS->GetHealthAttributeSet())
	{
		// RemoveAll before Add so a second call (OnRep_PlayerState after PossessedBy, or a later re-possession) can't stack duplicate bindings on a HealthAttributeSet that outlives any one avatar.
		HealthSet->OnHealthDepleted.RemoveAll(this);
		HealthSet->OnHealthDepleted.AddUObject(this, &ASOTMPlayerCharacter::HandleHealthDepleted);

		// Every (re)possession gets a full-health avatar, respawn included -- see ResetForRespawn's own comment for why a plain SetHealth from outside isn't enough.
		HealthSet->ResetForRespawn();
	}
}

void ASOTMPlayerCharacter::HandleProgressionChanged(const FProgressionSnapshot& Snapshot)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	UProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (ASC && Progression)
	{
		Progression->ReconcileAbilityGrants(ASC);
	}
}

void ASOTMPlayerCharacter::HandleHealthDepleted(AActor* OwnerActor)
{
	if (UGameFlowSubsystem* GameFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameFlowSubsystem>() : nullptr)
	{
		GameFlow->TryAcceptDeath(FGuid::NewGuid());
	}
}
