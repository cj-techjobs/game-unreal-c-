// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "SOTMIdentityTypes.h"
#include "SOTMCommandTypes.h"
#include "SOTMInteractable.generated.h"

/** One request to interact with a target: who, what, and which operation. */
USTRUCT(BlueprintType)
struct SOTMCORE_API FInteractionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	TWeakObjectPtr<AActor> Target;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	FGameplayTag Operation;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	FOperationId RequestId;
};

/**
 * A preview of what interacting would do. This is not authorization --
 * TryInteract revalidates distance, line of sight, state, and target
 * availability before acting on it.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FInteractionOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	bool bAvailable = false;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	FText Prompt;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	FGameplayTag Operation;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	float RequiredHoldSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Interaction")
	FGameplayTag UnavailableReason;
};

UINTERFACE(MinimalAPI)
class USOTMInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Capability for actors that can be interacted with (doors, chests,
 * stations, switches). Pure C++ contract: implemented by native classes,
 * not Blueprint graphs.
 */
class SOTMCORE_API ISOTMInteractable
{
	GENERATED_BODY()

public:
	virtual FInteractionOffer GetInteractionOffer(const FInteractionRequest& Request) const = 0;
	virtual FCommandResult TryInteract(const FInteractionRequest& Request) = 0;
	virtual void CancelInteraction(const FOperationId& RequestId) = 0;
};
