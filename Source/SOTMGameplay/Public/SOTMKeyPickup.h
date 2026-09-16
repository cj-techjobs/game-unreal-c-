// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMPersistable.h"
#include "SOTMKeyPickup.generated.h"

class USphereComponent;
class USOTMPersistenceComponent;

/**
 * A world key pickup (handbook Part 2 section 6.1's "Keys are inventory
 * facts with an access category/ID"). Structurally identical to
 * ASOTMCurrencyPickup -- same auto-collect-on-overlap, same
 * ISOTMPersistable-via-USOTMPersistenceComponent consumed-fact pattern,
 * same hide-rather-than-destroy reasoning -- grants a key count instead of
 * a wallet amount. Not merged into one shared base class for two pickup
 * types this small; revisit if a third auto-collect pickup appears.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMKeyPickup : public AActor, public ISOTMPersistable
{
	GENERATED_BODY()

public:
	ASOTMKeyPickup();

	virtual TArray<uint8> CapturePersistencePayload() const override;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) override;
	virtual void ResetPersistenceTransientState() override {}
	virtual bool IsReadyForPersistence() const override { return true; }

protected:
	/** Which lock category this grants access to (must match a chest/gate's RequiredAccessCategory). Defaults to a shared, non-empty category rather than NAME_None: an empty default would silently mean "every gate/chest with a matching empty default is already unlocked", the opposite of the intended safe default for an access-gating system. */
	UPROPERTY(EditAnywhere, Category = "SOTM|Pickup")
	FName AccessCategory = TEXT("Key.Common");

	UPROPERTY(EditAnywhere, Category = "SOTM|Pickup", meta = (ClampMin = "1"))
	int32 Quantity = 1;

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
