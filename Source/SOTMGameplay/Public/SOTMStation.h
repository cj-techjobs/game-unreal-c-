// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMInteractable.h"
#include "SOTMStation.generated.h"

class UBoxComponent;
class UUpgradeDefinition;

/**
 * A world purchase station (handbook Part 2 section 7's "station eligibility"
 * guard for TryPurchaseUpgrade -- this class is what finally gives that guard
 * something to be eligible near). Goes through ISOTMInteractable like
 * ASOTMChest/ASOTMGate rather than auto-collect, and needs the same
 * "BlockAll" BodyVolume for the same reason: USOTMInteractionComponent's
 * scanner only finds candidates with a collision-enabled component.
 *
 * Unlike a chest, a station is not ISOTMPersistable -- it has no local
 * one-shot state of its own to remember. Rank ownership already lives in
 * UProgressionSubsystem's persisted snapshot, so a station is safely
 * stateless and re-interactable forever; TryPurchaseUpgrade's own
 * already-owned-at-MaxRank/prerequisite/wallet checks are what make repeat
 * purchases fail once appropriate, not anything tracked here.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMStation : public AActor, public ISOTMInteractable
{
	GENERATED_BODY()

public:
	ASOTMStation();

	virtual FInteractionOffer GetInteractionOffer(const FInteractionRequest& Request) const override;
	virtual FCommandResult TryInteract(const FInteractionRequest& Request) override;
	virtual void CancelInteraction(const FOperationId& RequestId) override {}

protected:
	/** The single upgrade this station sells. Null means the station is unconfigured and offers nothing. */
	UPROPERTY(EditAnywhere, Category = "SOTM|Station")
	TObjectPtr<UUpgradeDefinition> UpgradeToSell;

	UPROPERTY(VisibleAnywhere, Category = "SOTM|Station")
	TObjectPtr<UBoxComponent> BodyVolume;
};
