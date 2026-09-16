// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMPersistable.h"
#include "SOTMCurrencyPickup.generated.h"

class USphereComponent;
class USOTMPersistenceComponent;

/**
 * "One real coin" (handbook M4: first complete gameplay loop). Auto-collects
 * on player overlap rather than going through ISOTMInteractable -- Part 2
 * section 6's interactable list (doors, chests, stations, notes, switches,
 * hiding spots, checkpoints) deliberately does not include bare pickups,
 * and ASOTMObjectiveMarker's proven Trigger-profile sphere is the right
 * pattern to reuse here instead of a second, heavier button-press flow.
 *
 * Persists "already collected" via ISOTMPersistable/USOTMPersistenceComponent
 * (the same pattern ASOTMWeaponPickup uses for bEquipped) rather than
 * giving UProgressionSubsystem::TryCollectPickup its own pickup-identity
 * ledger -- see that function's comment for why. This does mean the wallet
 * credit and the consumed-fact are two separate mutations (subsystem, then
 * actor) rather than the single atomic mutation the handbook's command
 * table literally describes -- a deliberate, documented deviation, not an
 * oversight.
 *
 * Unlike ASOTMWeaponPickup's ApplyPersistencePayload (which Destroy()s an
 * already-equipped weapon), this hides + disables collision instead of
 * destroying, on both live collection and restore. Destroying would
 * unregister it from UWorldStateSubsystem, so any *later* save in the same
 * session (e.g. from a subsequent purchase or death) would silently drop
 * this coin's record entirely -- the very next fresh level load would then
 * have nothing telling it "already collected", un-consuming it. Hiding in
 * place keeps it registered indefinitely, so the fact survives arbitrarily
 * many save/restore cycles. This is a real latent gap in the destroy-based
 * pattern, not something fixed here for ASOTMWeaponPickup -- out of scope
 * for one coin actor.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMCurrencyPickup : public AActor, public ISOTMPersistable
{
	GENERATED_BODY()

public:
	ASOTMCurrencyPickup();

	virtual TArray<uint8> CapturePersistencePayload() const override;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) override;
	virtual void ResetPersistenceTransientState() override {}
	virtual bool IsReadyForPersistence() const override { return true; }

protected:
	/** Credited to the wallet on collection (handbook Part 2 section 7: "the same configured currency ID"). */
	UPROPERTY(EditAnywhere, Category = "SOTM|Pickup", meta = (ClampMin = "0"))
	int64 RewardAmount = 10;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Pickup")
	TObjectPtr<USphereComponent> CollectionVolume;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Pickup")
	TObjectPtr<USOTMPersistenceComponent> PersistenceComponent;

private:
	bool bConsumed = false;

	void MarkConsumedAndHide();

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
