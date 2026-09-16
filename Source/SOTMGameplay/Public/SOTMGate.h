// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMInteractable.h"
#include "SOTMPersistable.h"
#include "SOTMGate.generated.h"

class UBoxComponent;
class USOTMPersistenceComponent;

/**
 * A world gate/door (handbook Part 1 section 6: "Door state and access
 * consumption follow the same rule [as TryOpenChest]... Once a gate is
 * retained as open, restore its collision/passage state directly even if
 * the consumed key is absent"). One BoxComponent serves as both the
 * physical obstacle and the interaction-scanner's discovery volume
 * (BlockAll while closed; NoCollision once open, matching "restore its
 * collision/passage state directly" -- an open gate is simply gone from
 * the physics world, not a still-detectable-but-passable trigger).
 *
 * Deliberately does not implement the encounter-seal half of section 6's
 * contract ("Separate a retained access/unlocked/open fact from a
 * temporary encounter seal... one mutable bIsOpen must not represent them
 * both") -- no boss/encounter system exists yet to seal anything, so a
 * single bOpen fact is honestly complete for this vertical slice, not a
 * simplification of something currently in use.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMGate : public AActor, public ISOTMInteractable, public ISOTMPersistable
{
	GENERATED_BODY()

public:
	ASOTMGate();

	virtual FInteractionOffer GetInteractionOffer(const FInteractionRequest& Request) const override;
	virtual FCommandResult TryInteract(const FInteractionRequest& Request) override;
	virtual void CancelInteraction(const FOperationId& RequestId) override {}

	virtual TArray<uint8> CapturePersistencePayload() const override;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) override;
	virtual void ResetPersistenceTransientState() override {}
	virtual bool IsReadyForPersistence() const override { return true; }

protected:
	/** Access category required to open this gate; NAME_None means no key required (a plain door). */
	UPROPERTY(EditAnywhere, Category = "SOTM|Gate")
	FName RequiredKeyCategory;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Gate")
	TObjectPtr<UBoxComponent> BodyVolume;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Gate")
	TObjectPtr<USOTMPersistenceComponent> PersistenceComponent;

private:
	bool bOpen = false;

	void SetOpenState(bool bNewOpen);
};
