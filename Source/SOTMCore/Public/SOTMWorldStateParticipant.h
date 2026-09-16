// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SOTMWorldStateTypes.h"
#include "SOTMWorldStateParticipant.generated.h"

UINTERFACE(MinimalAPI)
class USOTMWorldStateParticipant : public UInterface
{
	GENERATED_BODY()
};

/**
 * Capability for actors whose state persists across checkpoints/saves
 * (chests, gates, switches, consumed pickups). Pure C++ contract:
 * implemented by native classes, not Blueprint graphs.
 */
class SOTMCORE_API ISOTMWorldStateParticipant
{
	GENERATED_BODY()

public:
	virtual FGuid GetParticipantId() const = 0;
	virtual FWorldStateRecord CaptureWorldStateRecord() const = 0;
	virtual void ApplyWorldStateRecord(const FWorldStateRecord& Record) = 0;
	virtual void ResetTransientState() = 0;
	virtual bool IsReadyForWorldState() const = 0;
};
