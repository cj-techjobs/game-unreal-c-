// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SOTMWorldStateParticipant.h"
#include "SOTMPersistenceComponent.generated.h"

/**
 * Reusable ISOTMWorldStateParticipant: owns stable identity and
 * UWorldStateSubsystem registration so individual actors (weapon pickups,
 * chests, gates, switches) don't each reimplement that bookkeeping. The
 * actual payload -- what "equipped" or "opened" means -- comes from the
 * owning actor via ISOTMPersistable, since this component has no way to
 * know that itself.
 *
 * Not BlueprintSpawnableComponent: ISOTMPersistable, like
 * ISOTMWorldStateParticipant, is a native-only contract, so adding this to
 * a Blueprint-only actor would register a valid participant that always
 * captures an empty payload. Attach via CreateDefaultSubobject on a native
 * actor that implements ISOTMPersistable instead.
 *
 * Uses OnComponentCreated rather than the Actor-level OnConstruction
 * pattern ASOTMWeaponPickup used before this component existed: it fires
 * exactly once per real component instance -- for an editor-placed actor
 * and one spawned at runtime alike -- and not again on load, so no
 * WITH_EDITOR guard is needed here.
 */
UCLASS()
class SOTMGAMEPLAY_API USOTMPersistenceComponent : public UActorComponent, public ISOTMWorldStateParticipant
{
	GENERATED_BODY()

public:
	USOTMPersistenceComponent();

	virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual FGuid GetParticipantId() const override { return ParticipantId; }
	virtual FWorldStateRecord CaptureWorldStateRecord() const override;
	virtual void ApplyWorldStateRecord(const FWorldStateRecord& Record) override;
	virtual void ResetTransientState() override;
	virtual bool IsReadyForWorldState() const override;

protected:
	/** Assigned once when this component instance is first created (editor placement or runtime spawn); stable afterward. */
	UPROPERTY(VisibleAnywhere, Category = "SOTM|Persistence")
	FGuid ParticipantId;
};
