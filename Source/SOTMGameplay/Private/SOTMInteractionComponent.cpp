// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMInteractionComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Engine/Engine.h"

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

	// Debug-only on-screen prompt (no HUD widget exists yet). Re-printed every
	// scan tick (not gated on bChanged) with a duration just past the tick
	// interval so it stays continuously visible the whole time a target is in
	// range, using a fixed key so it overwrites in place rather than stacking.
	if (GEngine)
	{
		if (NewOffer.bAvailable)
		{
			GEngine->AddOnScreenDebugMessage(102, 0.3f, FColor::Green, FString::Printf(TEXT("[E] %s"), *NewOffer.Prompt.ToString()));
		}
		else
		{
			GEngine->RemoveOnScreenDebugMessage(102);
		}
	}
}

FCommandResult USOTMInteractionComponent::TryInteractWithCurrentTarget()
{
	FCommandResult Result;

	ISOTMInteractable* Interactable = Cast<ISOTMInteractable>(CurrentTargetActor);
	if (!Interactable)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(103, 3.0f, FColor::Red, TEXT("Interact: nothing in range"));
		}
		return Result;
	}

	FInteractionRequest Request;
	Request.Instigator = GetOwner();
	Request.Target = CurrentTargetActor;
	Request.RequestId = FOperationId::New();

	Result = Interactable->TryInteract(Request);

	if (GEngine)
	{
		const FString ResultMsg = Result.bSucceeded
			? TEXT("Interact: OK")
			: FString::Printf(TEXT("Interact: FAILED (%s)"), *Result.FailureReason.ToString());
		GEngine->AddOnScreenDebugMessage(103, 3.0f, Result.bSucceeded ? FColor::Green : FColor::Red, ResultMsg);
	}

	// Refresh immediately so the offer reflects the post-interact state (e.g. now unavailable) rather than waiting for the next tick.
	ScanForInteractable();

	return Result;
}
