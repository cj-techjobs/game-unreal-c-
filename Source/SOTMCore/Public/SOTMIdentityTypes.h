// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMIdentityTypes.generated.h"

/**
 * Identifies one playthrough/attempt. A new run gets a new ID on New Game
 * or New Run; it is what stable save records are scoped under.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FRunId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Identity")
	FGuid Value;

	FRunId() = default;
	explicit FRunId(const FGuid& InValue) : Value(InValue) {}

	bool IsValid() const { return Value.IsValid(); }
	static FRunId New() { return FRunId(FGuid::NewGuid()); }

	bool operator==(const FRunId& Other) const { return Value == Other.Value; }
	bool operator!=(const FRunId& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FRunId& Id)
{
	return GetTypeHash(Id.Value);
}

/**
 * Identifies one request made to a system (e.g. "try this purchase").
 * Lets a late/duplicate response be matched back to the request it
 * answers instead of being applied blindly.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FOperationId
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Identity")
	FGuid Value;

	FOperationId() = default;
	explicit FOperationId(const FGuid& InValue) : Value(InValue) {}

	bool IsValid() const { return Value.IsValid(); }
	static FOperationId New() { return FOperationId(FGuid::NewGuid()); }

	bool operator==(const FOperationId& Other) const { return Value == Other.Value; }
	bool operator!=(const FOperationId& Other) const { return !(*this == Other); }
};

FORCEINLINE uint32 GetTypeHash(const FOperationId& Id)
{
	return GetTypeHash(Id.Value);
}

/**
 * Increments each time a new level is loaded/travelled to. A captured
 * generation that no longer matches the current one marks its owning
 * async work as stale, so it can be discarded instead of acting on a
 * world that has already moved on.
 */
USTRUCT(BlueprintType)
struct SOTMCORE_API FWorldGeneration
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SOTM|Identity")
	int64 Value = 0;

	FWorldGeneration() = default;
	explicit FWorldGeneration(int64 InValue) : Value(InValue) {}

	FWorldGeneration Next() const { return FWorldGeneration(Value + 1); }

	bool operator==(const FWorldGeneration& Other) const { return Value == Other.Value; }
	bool operator!=(const FWorldGeneration& Other) const { return !(*this == Other); }
};
