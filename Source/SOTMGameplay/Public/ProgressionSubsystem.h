// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SOTMProgressionTypes.h"
#include "SOTMCommandTypes.h"
#include "ProgressionSubsystem.generated.h"

class UUpgradeDefinition;
class UAbilityDefinition;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressionChanged, const FProgressionSnapshot&, Snapshot);

/**
 * Owns the active run ledger. Inventory/ObjectiveRecords are still
 * deferred (see FProgressionSnapshot); OwnedUpgradeRanks and
 * TryPurchaseUpgrade are real. Per section 8.3's save-trigger list, commits
 * a persisted snapshot via USaveSubsystem after death acceptance and
 * purchases (debits); trivial credits are intentionally NOT autosaved per
 * "coalesce trivial pickup saves".
 *
 * TryPurchaseUpgrade only validates what's implementable today: current
 * rank vs MaxRank, prerequisite ranks, and wallet balance. Station
 * eligibility and build availability (handbook Part 2 section 7.1's other
 * two guards) still need systems that don't exist yet (station actors,
 * UBuildProfileDefinition) and are deferred rather than half-simulated.
 * "Loaded/validated grant" is now real via ReconcileAbilityGrants, but only
 * reconciliation (idempotent re-grant against the current ASC) -- it does
 * not yet pre-validate that a purchase's GrantedAbilityId resolves to a
 * loadable AbilityClass before committing the debit, so a dangling/unset
 * GrantedAbilityId still lets the purchase succeed with no ability granted.
 */
UCLASS()
class SOTMGAMEPLAY_API UProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Broadcast after every committed change, once the new revision is applied. */
	UPROPERTY(BlueprintAssignable, Category = "SOTM|Progression")
	FOnProgressionChanged OnProgressionChanged;

	/** Resets the ledger to a fresh run: new RunId, five lives, empty wallet. Does not autosave -- there is nothing worth persisting yet. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	void StartNewRun();

	/**
	 * Replaces the ledger with a previously-saved snapshot (handbook Part 1
	 * section 8.4's restoration sequence) and broadcasts
	 * OnProgressionChanged so reactive listeners refresh against the
	 * restored state: UObjectiveSubsystem re-evaluates HaveCurrency/
	 * OwnUpgrade conditions, ASOTMPlayerCharacter reconciles ability grants
	 * if an avatar is currently possessed, and any HUD presenter re-reads
	 * the snapshot. Does not autosave -- restoring is not itself a new
	 * change worth persisting again.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	void RestoreSnapshot(const FProgressionSnapshot& RestoredSnapshot);

	UFUNCTION(BlueprintPure, Category = "SOTM|Progression")
	FProgressionSnapshot GetSnapshot() const { return Snapshot; }

	/** Consumes one life. Fails if the run is already over/completed. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryAcceptDeath();

	/** Fails if Amount is negative. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryCreditWallet(int64 Amount);

	/**
	 * The handbook Part 2 section 7.1 pickup-collection command: fails if
	 * the run isn't in progress or RewardAmount is negative, otherwise
	 * credits the wallet. Unlike TryPurchaseUpgrade, this does not itself
	 * track pickup identity/"already consumed" -- UWorldStateSubsystem's
	 * per-placement record already solves that (handbook Part 1 section
	 * 6.2's "collection deduplication uses each placement's persistent
	 * ID"), so a second parallel ledger here would duplicate it. The
	 * calling pickup actor (see ASOTMCurrencyPickup) is responsible for
	 * checking its own already-consumed state before calling this and for
	 * recording the consumed fact afterward via ISOTMPersistable. Coalesced
	 * with other trivial credits per the class comment -- does not autosave.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryCollectPickup(int64 RewardAmount);

	/** Fails if the run isn't in progress or Quantity isn't positive. Used by a key pickup, mirroring TryCollectPickup's identity split: the pickup actor owns its own already-consumed fact via ISOTMPersistable, this only owns the fungible count. Coalesced like other trivial credits -- does not autosave. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryGrantKey(FName AccessCategory, int32 Quantity);

	/** Fails if the run isn't in progress or fewer than one key of AccessCategory is owned. Decrements the count as one atomic mutation. Meaningful enough (a door permanently opened) to autosave, unlike trivial credits. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryConsumeKey(FName AccessCategory);

	/**
	 * Handbook Part 2 section 6.1/7.1's TryOpenChest: validates the run is
	 * active, RewardAmount is nonnegative, and (if RequiredAccessCategory is
	 * not NAME_None) at least one matching key is owned, then commits key
	 * consumption and reward grant as one atomic mutation -- "do not
	 * consume the key and then discover the reward cannot be granted" is
	 * satisfied by validating both before mutating either. Like
	 * TryCollectPickup, does not itself track "is this chest already
	 * opened" -- the calling chest actor owns that fact via ISOTMPersistable
	 * (see ASOTMChest), for the same reason UWorldStateSubsystem's
	 * per-placement record already solves it.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryOpenChest(FName RequiredAccessCategory, int64 RewardAmount);

	/** Fails if Amount isn't positive or exceeds the current balance. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryDebitWallet(int64 Amount);

	/** Debits wallet, advances rank, and records ownership as one atomic mutation. Fails (with a typed FailureReason) if Definition is null/malformed, already at MaxRank, missing a prerequisite rank, or the wallet can't cover FlatPrice. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	FCommandResult TryPurchaseUpgrade(UUpgradeDefinition* Definition);

	/** Makes Definition resolvable by ContentId for later ability-grant reconciliation. TryPurchaseUpgrade calls this for whatever it's given, so a definition is known as soon as it's ever purchased with; call directly to register one before a run starts (e.g. at bootstrap) so a restored save can reconcile grants immediately. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	void RegisterUpgradeDefinition(UUpgradeDefinition* Definition);

	/** Makes Definition resolvable by ContentId (matched against UUpgradeDefinition::GrantedAbilityId's tag name, the same bridging convention UObjectiveDefinition::ConditionTargetTag uses against UUpgradeDefinition::ContentId) so ReconcileAbilityGrants can find its AbilityClass. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	void RegisterAbilityDefinition(UAbilityDefinition* Definition);

	/**
	 * Grants an ability for every currently-owned upgrade rank whose
	 * GrantedAbilityId resolves to a registered UAbilityDefinition, skipping
	 * any AbilityClass already present on AbilitySystemComponent. Idempotent
	 * by construction (checked against the ASC itself, not a separate handle
	 * ledger -- there is exactly one live avatar/ASC per PlayerState in this
	 * single-player game, so the ASC's own granted-ability list is already
	 * the source of truth) -- safe to call on every possession and every
	 * progression change. This is the "reconcile the ASC from committed
	 * ownership" step handbook Part 2 section 7.1 and the persistence
	 * section both call for; it does not track live handles anywhere.
	 */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Progression")
	void ReconcileAbilityGrants(UAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY()
	FProgressionSnapshot Snapshot;

	UPROPERTY()
	TMap<FName, TObjectPtr<UUpgradeDefinition>> UpgradeDefinitionsByContentId;

	UPROPERTY()
	TMap<FName, TObjectPtr<UAbilityDefinition>> AbilityDefinitionsByContentId;

	/** Bumps the revision and broadcasts the new snapshot. */
	void Commit();

	/** Bumps the revision, broadcasts, and pushes a save envelope (see class comment for which commands call this vs. plain Commit). */
	void CommitAndSave();
};
