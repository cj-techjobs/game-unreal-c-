// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SOTMObjectiveTarget.h"
#include "SOTMObjectiveMarker.generated.h"

class USphereComponent;

/**
 * Placeable "named area" (handbook Part 1 section 14's objective triggers /
 * named areas). Reports EnterArea condition events to UObjectiveSubsystem
 * on player overlap -- it does not know or care which objective, if any,
 * is listening; ISOTMObjectiveTarget's whole point is that an objective
 * targets this marker's MarkerId tag, not this actor class.
 *
 * "Valid player identity" (handbook Part 2 section 8's EnterArea
 * evaluation) is checked via APawn::IsPlayerControlled() rather than
 * casting to a concrete pawn class -- this only needs to know "is this the
 * player," which the engine already answers, not which project-specific
 * character class it is.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMObjectiveMarker : public AActor, public ISOTMObjectiveTarget
{
	GENERATED_BODY()

public:
	ASOTMObjectiveMarker();

	virtual FGameplayTag GetObjectiveMarkerId() const override { return MarkerId; }

protected:
	/** What ISOTMObjectiveDefinition::ConditionTargetTag should reference to target this marker. EditAnywhere (not EditDefaultsOnly): each placed marker is a distinct named area and needs its own id, the same class placed twice must not share one. */
	UPROPERTY(EditAnywhere, Category = "SOTM|Objective")
	FGameplayTag MarkerId;

	/** SphereRadius is already EditAnywhere on the component itself -- no separate wrapper field, so there's nothing to fall out of sync with it. */
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Objective")
	TObjectPtr<USphereComponent> TriggerVolume;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
