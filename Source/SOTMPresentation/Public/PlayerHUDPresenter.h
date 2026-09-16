// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMProgressionTypes.h"
#include "SOTMObjectiveTypes.h"
#include "PlayerHUDPresenter.generated.h"

class ULocalPlayer;
class UObjectiveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHUDSnapshotChanged);

/**
 * Typed, event-driven snapshot of HUD-relevant state (handbook Part 2
 * section 13: "Typed presenter objects expose immutable/controlled fields
 * and delegates... Widgets bind when activated and detach when
 * deactivated/destroyed. They fetch a current snapshot..."). Owned by
 * UPresentationSubsystem, one per local player.
 *
 * Only covers what's already backed by a real subsystem: run/wallet state
 * (UProgressionSubsystem) and active objectives (UObjectiveSubsystem).
 * Health/stamina/ability slots (no GAS/ASC yet), upgrade cost/shortfall and
 * available ranks (no UUpgradeDefinition yet), interaction prompt/map
 * markers/boss progress, and dialogue/subtitles/input glyphs/accessibility/
 * save status are all deferred until their owning systems exist.
 *
 * UProgressionSubsystem is a GameInstance subsystem (stable for the whole
 * session), so binding to its delegate once in InitializeForLocalPlayer is
 * safe. UObjectiveSubsystem is a World subsystem -- a new one exists after
 * every map travel -- so RefreshSnapshot() re-resolves it and rebinds if it
 * changed, rather than trusting a subsystem pointer cached once at
 * initialization time (handbook Part 1 section 4.2's "local-player
 * services detach old presenters during travel").
 */
UCLASS(BlueprintType)
class SOTMPRESENTATION_API UPlayerHUDPresenter : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "SOTM|Presentation")
	FOnHUDSnapshotChanged OnHUDSnapshotChanged;

	/** Binds to the owning local player's UProgressionSubsystem; call once from UPresentationSubsystem::Initialize. */
	void InitializeForLocalPlayer(ULocalPlayer* InLocalPlayer);

	/** Unbinds from both subsystems; call from UPresentationSubsystem::Deinitialize. */
	void Shutdown();

	/** Re-resolves the current world's UObjectiveSubsystem (rebinding if it changed since last call) and re-pulls both snapshots. Widgets should call this once on activation so they reflect state that changed while they were closed. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Presentation")
	void RefreshSnapshot();

	UFUNCTION(BlueprintPure, Category = "SOTM|Presentation")
	FProgressionSnapshot GetProgressionSnapshot() const { return CachedProgression; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Presentation")
	TArray<FObjectiveState> GetActiveObjectives() const { return CachedObjectives; }

private:
	UPROPERTY()
	TWeakObjectPtr<ULocalPlayer> OwningLocalPlayer;

	UPROPERTY()
	TWeakObjectPtr<UObjectiveSubsystem> BoundObjectiveSubsystem;

	UPROPERTY()
	FProgressionSnapshot CachedProgression;

	UPROPERTY()
	TArray<FObjectiveState> CachedObjectives;

	UFUNCTION()
	void HandleProgressionChanged(const FProgressionSnapshot& Snapshot);

	UFUNCTION()
	void HandleObjectiveTransitioned(const FObjectiveState& State);

	void EnsureObjectiveBinding();
	UObjectiveSubsystem* ResolveObjectiveSubsystem() const;
};
