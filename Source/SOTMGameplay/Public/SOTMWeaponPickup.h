// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMInteractable.h"
#include "SOTMPersistable.h"
#include "SOTMWeaponPickup.generated.h"

class USOTMPersistenceComponent;

/**
 * A world weapon pickup. Attaches itself to the interacting player on
 * interact rather than granting an inventory item -- there is no
 * inventory/equip system yet, so this is an honest, minimal stand-in
 * (visually equips, nothing more) rather than a guess at one.
 *
 * Ported from BP_WEAPON, which implemented the legacy BPI_Interaction
 * Blueprint interface's "Interact" event: cast the local player to
 * BP_Speedster (a leftover class from an unrelated prototype, not the
 * project's actual player), call "EquipWeapon" on it, then destroy self.
 * That cast always fails against the real player (BP_ThirdPersonCharacter/
 * ASOTMPlayerCharacter), so the pickup has never actually worked in the
 * live game -- this is a fix, not just a port. Also found unreferenced by
 * any level or asset (get_referencers returned nothing), so it may not be
 * placed anywhere yet either.
 *
 * Uses the native ISOTMInteractable contract instead of BPI_Interaction,
 * and checks for ASOTMPlayerCharacter directly rather than adding a new
 * single-purpose interface for "can equip weapons" -- one call site with
 * no other planned implementer doesn't justify that abstraction yet.
 *
 * Persistence ("consumed pickup stays consumed", handbook Part 1 section
 * 8.1 -- bEquipped must survive a checkpoint/save reload, not just live in
 * memory for the session) is delegated to a USOTMPersistenceComponent
 * rather than implemented inline: the component owns identity and
 * UWorldStateSubsystem registration, this class only supplies the
 * bEquipped payload via ISOTMPersistable.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMWeaponPickup : public AActor, public ISOTMInteractable, public ISOTMPersistable
{
	GENERATED_BODY()

public:
	ASOTMWeaponPickup();

	virtual FInteractionOffer GetInteractionOffer(const FInteractionRequest& Request) const override;
	virtual FCommandResult TryInteract(const FInteractionRequest& Request) override;
	virtual void CancelInteraction(const FOperationId& RequestId) override;

	virtual TArray<uint8> CapturePersistencePayload() const override;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) override;
	virtual void ResetPersistenceTransientState() override {}
	virtual bool IsReadyForPersistence() const override { return true; }

protected:
	/** Socket on the wielder's mesh to attach to; falls back to the root if it doesn't exist. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Weapon")
	FName WeaponSocketName = "WeaponSocket";

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Weapon")
	TObjectPtr<USOTMPersistenceComponent> PersistenceComponent;

private:
	bool bEquipped = false;
};
