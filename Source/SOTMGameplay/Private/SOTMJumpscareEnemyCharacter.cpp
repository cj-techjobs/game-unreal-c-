// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMJumpscareEnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"

ASOTMJumpscareEnemyCharacter::ASOTMJumpscareEnemyCharacter()
{
}

void ASOTMJumpscareEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UBoxComponent* Box = FindComponentByClass<UBoxComponent>())
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &ASOTMJumpscareEnemyCharacter::HandleTouchOverlap);
	}
}

void ASOTMJumpscareEnemyCharacter::HandleTouchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OtherPawn->GetController());
	if (PC)
	{
		PC->SetViewTargetWithBlend(this, CameraBlendSeconds);
	}

	if (JumpscareAnim && GetMesh())
	{
		GetMesh()->PlayAnimation(JumpscareAnim, false);
	}

	if (PC)
	{
		OtherPawn->DisableInput(PC);
	}
}
