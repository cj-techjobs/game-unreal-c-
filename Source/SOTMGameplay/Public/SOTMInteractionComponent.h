// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SOTMInteractable.h"
#include "SOTMInteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionOfferChanged, const FInteractionOffer&, Offer);

/**
 * The "interaction scanner" the handbook's Part 2 section 1 says
 * ASOTMPlayerCharacter contains. Every tick (throttled, not per-frame --
 * see the "no global actor scans" performance rule), does a bounded
 * sphere overlap around the owner to find the nearest actor implementing
 * ISOTMInteractable, and keeps a current FInteractionOffer for a HUD
 * prompt to read via OnInteractionOfferChanged.
 *
 * Matches the handbook's IA_Interact contract ("Press/hold from current
 * interaction offer | Interaction component"): the owning character binds
 * IA_Interact to TryInteractWithCurrentTarget(), not to a raw trace of its
 * own.
 */
UCLASS(ClassGroup = (SOTM), meta = (BlueprintSpawnableComponent))
class SOTMGAMEPLAY_API USOTMInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USOTMInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Interaction")
	FOnInteractionOfferChanged OnInteractionOfferChanged;

	/** Builds a fresh request against the current best target and calls its TryInteract. Fails (no-op) if nothing is currently targeted. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Interaction")
	FCommandResult TryInteractWithCurrentTarget();

	UFUNCTION(BlueprintPure, Category = "SOTM|Interaction")
	FInteractionOffer GetCurrentOffer() const { return CurrentOffer; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Interaction")
	AActor* GetCurrentTarget() const { return CurrentTargetActor; }

protected:
	/** Radius of the bounded overlap used to find candidate interactables around the owner. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Interaction")
	float ScanRadius = 200.0f;

private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

	FInteractionOffer CurrentOffer;

	void ScanForInteractable();
};
