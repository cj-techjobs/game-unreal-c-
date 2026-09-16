// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMWorldStateTypes.generated.h"

/**
 * Saved record for one persistent world participant (a chest, a door, a
 * switch). A chest and a door save different facts, so this contract only
 * fixes identity + envelope; Payload is serialized/read by each concrete
 * participant, not interpreted here.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FWorldStateRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|World")
	FGuid ParticipantId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|World")
	TArray<uint8> Payload;
};
