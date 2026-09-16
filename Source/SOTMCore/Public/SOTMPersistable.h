// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SOTMPersistable.generated.h"

UINTERFACE(MinimalAPI)
class USOTMPersistable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by the owner of a USOTMPersistenceComponent to supply the
 * actual state to persist. The component owns identity and
 * UWorldStateSubsystem registration (the reusable, boilerplate half of
 * ISOTMWorldStateParticipant); it has no way to know what "equipped" or
 * "opened" means for a given actor, so it forwards through this contract
 * instead. Pure C++ contract: implemented by native classes, not
 * Blueprint graphs.
 */
class SOTMCORE_API ISOTMPersistable
{
	GENERATED_BODY()

public:
	virtual TArray<uint8> CapturePersistencePayload() const = 0;
	virtual void ApplyPersistencePayload(const TArray<uint8>& Payload) = 0;
	virtual void ResetPersistenceTransientState() = 0;
	virtual bool IsReadyForPersistence() const = 0;
};
