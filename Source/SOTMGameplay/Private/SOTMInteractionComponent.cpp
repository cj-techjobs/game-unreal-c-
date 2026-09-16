// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMInteractionComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

USOTMInteractionComponent::USOTMInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void USOTMInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ScanForInteractable();
}

void USOTMInteractionComponent::ScanForInteractable()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(ScanRadius);
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
		Sphere,
		FCollisionQueryParams(NAME_None, false, Owner));

	AActor* BestCandidate = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || !Candidate->Implements<USOTMInteractable>())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestCandidate = Candidate;
		}
	}

	FInteractionOffer NewOffer;
	if (ISOTMInteractable* Interactable = Cast<ISOTMInteractable>(BestCandidate))
	{
		FInteractionRequest Request;
		Request.Instigator = Owner;
		Request.Target = BestCandidate;
		NewOffer = Interactable->GetInteractionOffer(Request);
	}

	const bool bChanged = (BestCandidate != CurrentTargetActor) || (NewOffer.bAvailable != CurrentOffer.bAvailable);

	CurrentTargetActor = BestCandidate;
	CurrentOffer = NewOffer;

	if (bChanged)
	{
		OnInteractionOfferChanged.Broadcast(CurrentOffer);
	}
}

FCommandResult USOTMInteractionComponent::TryInteractWithCurrentTarget()
{
	FCommandResult Result;

	ISOTMInteractable* Interactable = Cast<ISOTMInteractable>(CurrentTargetActor);
	if (!Interactable)
	{
		return Result;
	}

	FInteractionRequest Request;
	Request.Instigator = GetOwner();
	Request.Target = CurrentTargetActor;
	Request.RequestId = FOperationId::New();

	Result = Interactable->TryInteract(Request);

	// Refresh immediately so the offer reflects the post-interact state (e.g. now unavailable) rather than waiting for the next tick.
	ScanForInteractable();

	return Result;
}
