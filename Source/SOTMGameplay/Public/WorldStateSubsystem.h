// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SOTMWorldStateParticipant.h"
#include "WorldStateSubsystem.generated.h"

/**
 * Persistent participant registry; applies retained/reset records to
 * loaded actors. Gameplay-only: excluded from editor preview worlds so it
 * never activates outside Game/PIE.
 *
 * Per the handbook's Part 1 section 8.4 restoration sequence: retained
 * records apply to participants already registered, and are held for any
 * ParticipantId with no live match yet so "streamed participants register
 * later and immediately receive the correct record for this generation."
 *
 * Registry storage and the public API use plain UObject pointers /
 * TWeakObjectPtr rather than TScriptInterface<ISOTMWorldStateParticipant>: the interface
 * is deliberately NotBlueprintable (native-only contract), and a
 * TScriptInterface of a non-Blueprintable interface silently breaks
 * Blueprint exposure for every UFUNCTION on this class, not just the ones
 * that take it as a parameter -- found by trying to call these from a
 * test Blueprint and seeing none of them, including ones with no
 * interface parameter at all, show up. Cast<ISOTMWorldStateParticipant>
 * internally instead; callers just pass the object.
 */
UCLASS()
class SOTMGAMEPLAY_API UWorldStateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Participant must implement ISOTMWorldStateParticipant; ignored otherwise. Also immediately applies any pending retained record queued for its ID. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|WorldState")
	void RegisterParticipant(UObject* Participant);

	UFUNCTION(BlueprintCallable, Category = "SOTM|WorldState")
	void UnregisterParticipant(UObject* Participant);

	/** One record per currently-registered participant that reports ready. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|WorldState")
	TArray<FWorldStateRecord> CaptureAllRecords() const;

	/** Applies to already-registered participants now; unmatched records wait for that participant to register (see RegisterParticipant). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|WorldState")
	void ApplyRetainedRecords(const TArray<FWorldStateRecord>& Records);

	UFUNCTION(BlueprintCallable, Category = "SOTM|WorldState")
	void ResetAllTransientState();

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> ParticipantObjects;

	TMap<FGuid, FWorldStateRecord> PendingRecordsByParticipantId;
};
