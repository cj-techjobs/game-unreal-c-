// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMWeaponPickup.h"
#include "SOTMPlayerCharacter.h"
#include "SOTMPersistenceComponent.h"
#include "Components/SkeletalMeshComponent.h"

ASOTMWeaponPickup::ASOTMWeaponPickup()
{
	PersistenceComponent = CreateDefaultSubobject<USOTMPersistenceComponent>(TEXT("PersistenceComponent"));
}

TArray<uint8> ASOTMWeaponPickup::CapturePersistencePayload() const
{
	return { bEquipped ? uint8(1) : uint8(0) };
}

void ASOTMWeaponPickup::ApplyPersistencePayload(const TArray<uint8>& Payload)
{
	// Already taken in a prior session/checkpoint: there's no live wielder to
	// re-attach to and no player-side inventory to restore into yet, so the
	// honest move is to remove the pickup rather than pretend it's still here.
	if (Payload.Num() > 0 && Payload[0] != 0)
	{
		Destroy();
	}
}

FInteractionOffer ASOTMWeaponPickup::GetInteractionOffer(const FInteractionRequest& Request) const
{
	FInteractionOffer Offer;
	Offer.bAvailable = !bEquipped;
	Offer.Operation = Request.Operation;
	Offer.Prompt = NSLOCTEXT("SOTM", "PickUpWeapon", "Pick Up Weapon");
	return Offer;
}

FCommandResult ASOTMWeaponPickup::TryInteract(const FInteractionRequest& Request)
{
	FCommandResult Result;
	Result.OperationId = Request.RequestId;

	if (bEquipped)
	{
		return Result;
	}

	ASOTMPlayerCharacter* Wielder = Cast<ASOTMPlayerCharacter>(Request.Instigator.Get());
	if (!Wielder)
	{
		return Result;
	}

	if (USkeletalMeshComponent* WielderMesh = Wielder->GetMesh())
	{
		const FName SocketToUse = WielderMesh->DoesSocketExist(WeaponSocketName) ? WeaponSocketName : NAME_None;
		AttachToComponent(WielderMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketToUse);
	}
	else
	{
		AttachToActor(Wielder, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	SetActorEnableCollision(false);
	bEquipped = true;
	Result.bSucceeded = true;
	return Result;
}

void ASOTMWeaponPickup::CancelInteraction(const FOperationId& RequestId)
{
}
