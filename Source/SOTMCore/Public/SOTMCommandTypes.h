// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SOTMIdentityTypes.h"
#include "SOTMCommandTypes.generated.h"

/**
 * Generic answer to a command (a request that asks an owning system to
 * change state). FailureReason is a tag rather than free text so calling
 * code can branch on it instead of parsing a message.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FCommandResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Result")
	bool bSucceeded = false;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Result")
	FOperationId OperationId;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Result")
	int64 Revision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Result")
	FGameplayTag FailureReason;
};
