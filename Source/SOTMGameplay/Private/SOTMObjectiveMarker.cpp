// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMObjectiveMarker.h"
#include "Components/SphereComponent.h"
#include "ObjectiveSubsystem.h"
#include "GameFramework/Pawn.h"

ASOTMObjectiveMarker::ASOTMObjectiveMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerVolume"));
	TriggerVolume->InitSphereRadius(150.0f);

	// The engine's own "Trigger" profile (object type WorldDynamic, Pawn
	// response Overlap) rather than hand-rolled responses -- verified via a
	// live PIE test that a possessed DefaultPawn swept into this volume
	// generates BeginOverlap correctly with this profile.
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASOTMObjectiveMarker::HandleBeginOverlap);
	SetRootComponent(TriggerVolume);
}

void ASOTMObjectiveMarker::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled() || !MarkerId.IsValid())
	{
		return;
	}

	if (UObjectiveSubsystem* Objectives = GetWorld()->GetSubsystem<UObjectiveSubsystem>())
	{
		Objectives->ReportAreaEntered(MarkerId);
	}
}
