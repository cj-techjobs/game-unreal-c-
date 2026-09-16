// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMInteractable.h"
#include "SOTMPersistable.h"
#include "SOTMChest.generated.h"

class UBoxComponent;
class USOTMPersistenceComponent;

/**
 * A world chest (handbook Part 1 section 6/6.1's TryOpenChest). Unlike the
 * auto-collect pickups, chests are in section 6's explicit interactable
 * list, so this goes through ISOTMInteractable -- USOTMInteractionComponent
 * (already built and already bound to IA_Interact on ASOTMPlayerCharacter)
 * discovers and offers it with no new plumbing needed on the player side.
 *
 * Needs a real collision component (BodyVolume, "BlockAll" profile): the
 * interaction scanner finds candidates via
 * UWorld::OverlapMultiByObjectType(AllObjects) around the player, a real
 * physics query -- an actor with no collision-enabled component is
 * invisible to it regardless of implementing ISOTMInteractable. BlockAll
 * both makes the chest a solid obstacle (expected chest behavior) and
 * keeps it query-discoverable.
 *
 * "Opened" persists via ISOTMPersistable/USOTMPersistenceComponent, same
 * pattern as ASOTMWeaponPickup's bEquipped -- but hides nothing and
 * destroys nothing on restore (unlike the weapon): a chest has no
 * elsewhere-to-go semantic, it just becomes permanently non-interactable,
 * so ApplyPersistencePayload only needs to flip bOpened, matching
 * ASOTMCurrencyPickup's more-robust choice over the weapon's.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMChest : public AActor, public ISOTMInteractable, public ISOTMPersistable
{
	GENERATED_BODY()

public:
	ASOTMChest();

	virtual FInteractionOffer GetInteractionOffer(const FInteractionRequest& Request) const override;
	virtual FCommandResult TryInteract(const FInteractionRequest& Request) override;
	virtual void CancelInteraction(const FOperationId& RequestId) override {}

	virtual TArray<uint8> CapturePersistencePayload() const override;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) override;
	virtual void ResetPersistenceTransientState() override {}
	virtual bool IsReadyForPersistence() const override { return true; }

protected:
	/** Access category required to open this chest; NAME_None means no key required. */
	UPROPERTY(EditAnywhere, Category = "SOTM|Chest")
	FName RequiredKeyCategory;

	UPROPERTY(EditAnywhere, Category = "SOTM|Chest", meta = (ClampMin = "0"))
	int64 RewardAmount = 25;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Chest")
	TObjectPtr<UBoxComponent> BodyVolume;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Chest")
	TObjectPtr<USOTMPersistenceComponent> PersistenceComponent;

private:
	bool bOpened = false;
};
