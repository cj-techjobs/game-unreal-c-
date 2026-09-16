// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "SOTMCommandTypes.h"
#include "ChapterSubsystem.generated.h"

class UChapterDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChapterReady);

/**
 * Active chapter identity, readiness barrier, and checkpoint restore
 * orchestration. Gameplay-only: excluded from editor preview worlds so it
 * never activates outside Game/PIE.
 *
 * ActivateChapterFromDefinition reads a UChapterDefinition's ChapterId and
 * ObjectiveRootIds only -- required-asset bundle loading and encounter
 * roster activation stay deferred because those definition fields don't
 * exist yet (no Asset Manager bundle plumbing, no UEncounterDefinition).
 * It also does not itself grant abilities from owned upgrades (GAS exists
 * now, but that reconciliation needs a live ASC, which this chapter-scoped
 * call has no reason to assume exists yet). Callers can still drive
 * readiness requirements and restoration explicitly, as before.
 *
 * RestoreFromLatestSave() now covers world records, objective transients,
 * and the progression ledger (wallet/lives/upgrade ranks) -- restoring
 * progression is done last, since UProgressionSubsystem::RestoreSnapshot's
 * broadcast is what makes UObjectiveSubsystem re-evaluate HaveCurrency/
 * OwnUpgrade conditions and (if an avatar is currently possessed)
 * ASOTMPlayerCharacter reconcile ability grants against the restored
 * ranks; doing it before the objective reset above would have that
 * re-evaluation immediately clobbered. Nav queries (validate spawn) are
 * still not implemented.
 */
UCLASS()
class SOTMGAMEPLAY_API UChapterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Fires once per ActivateChapter call, the first time every added readiness requirement is satisfied. */
	UPROPERTY(BlueprintAssignable, Category = "SOTM|Chapter")
	FOnChapterReady OnChapterReady;

	/** Marks a chapter active and clears the readiness barrier for it. Objective/encounter services are expected to stay dormant until this is called. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	void ActivateChapter(FGameplayTag ChapterId);

	/** Convenience over ActivateChapter: also remembers Definition and activates each of its ObjectiveRootIds via UObjectiveSubsystem. No-op if Definition is null. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	void ActivateChapterFromDefinition(UChapterDefinition* Definition);

	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	void DeactivateChapter();

	UFUNCTION(BlueprintPure, Category = "SOTM|Chapter")
	bool IsChapterActive() const { return ActiveChapterId.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "SOTM|Chapter")
	FGameplayTag GetActiveChapterId() const { return ActiveChapterId; }

	/** Null unless the active chapter was entered via ActivateChapterFromDefinition. */
	UFUNCTION(BlueprintPure, Category = "SOTM|Chapter")
	UChapterDefinition* GetActiveChapterDefinition() const { return ActiveChapterDefinition; }

	/** Adds a named requirement that must be satisfied before IsReady() returns true. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	void AddReadinessRequirement(FGameplayTag RequirementId);

	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	void SatisfyReadinessRequirement(FGameplayTag RequirementId);

	UFUNCTION(BlueprintPure, Category = "SOTM|Chapter")
	bool IsReady() const { return OutstandingRequirements.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "SOTM|Chapter")
	TArray<FGameplayTag> GetOutstandingRequirements() const;

	/** Loads the latest valid save and applies its retained world records / resets objective transients. Fails if there's no valid save (expected for a fresh New Game, which shouldn't call this). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Chapter")
	FCommandResult RestoreFromLatestSave();

private:
	UPROPERTY()
	FGameplayTag ActiveChapterId;

	UPROPERTY()
	TObjectPtr<UChapterDefinition> ActiveChapterDefinition;

	UPROPERTY()
	TSet<FGameplayTag> OutstandingRequirements;

	bool bReadyBroadcast = false;
};
