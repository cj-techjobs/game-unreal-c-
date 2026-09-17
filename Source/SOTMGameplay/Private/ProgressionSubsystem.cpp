// Fill out your copyright notice in the Description page of Project Settings.

#include "ProgressionSubsystem.h"
#include "SaveSubsystem.h"
#include "WorldStateSubsystem.h"
#include "UpgradeDefinition.h"
#include "AbilityDefinition.h"
#include "SOTMResultTags.h"
#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UProgressionSubsystem::Commit()
{
	++Snapshot.Revision;
	OnProgressionChanged.Broadcast(Snapshot);
}

void UProgressionSubsystem::CommitAndSave()
{
	Commit();

	UGameInstance* GameInstance = GetGameInstance();
	USaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<USaveSubsystem>() : nullptr;
	if (!SaveSubsystem)
	{
		return;
	}

	// Every prior call here saved an empty world-state array regardless of what UWorldStateSubsystem actually held -- a save never really captured chest/pickup/gate state even though CaptureAllRecords() was real. Capture it for real now.
	UWorld* World = GameInstance->GetWorld();
	UWorldStateSubsystem* WorldState = World ? World->GetSubsystem<UWorldStateSubsystem>() : nullptr;
	const TArray<FWorldStateRecord> Records = WorldState ? WorldState->CaptureAllRecords() : TArray<FWorldStateRecord>();

	SaveSubsystem->SaveSnapshot(Snapshot, Records, {});
}

void UProgressionSubsystem::StartNewRun()
{
	Snapshot = FProgressionSnapshot();
	Snapshot.RunId = FRunId::New();
	Snapshot.RemainingLives = 5;
	Snapshot.Wallet = 0;
	Snapshot.RunStatus = ERunStatus::InProgress;
	Commit();
}

void UProgressionSubsystem::RestoreSnapshot(const FProgressionSnapshot& RestoredSnapshot)
{
	Snapshot = RestoredSnapshot;
	Commit();
}

FCommandResult UProgressionSubsystem::TryAcceptDeath()
{
	FCommandResult Result;

	if (Snapshot.RunStatus != ERunStatus::InProgress)
	{
		return Result;
	}

	Snapshot.RemainingLives = FMath::Max(0, Snapshot.RemainingLives - 1);
	if (Snapshot.RemainingLives == 0)
	{
		Snapshot.RunStatus = ERunStatus::GameOver;
	}

	CommitAndSave();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryCreditWallet(int64 Amount)
{
	FCommandResult Result;

	if (Amount < 0)
	{
		return Result;
	}

	Snapshot.Wallet += Amount;
	Commit();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryCollectPickup(int64 RewardAmount)
{
	FCommandResult Result;

	if (Snapshot.RunStatus != ERunStatus::InProgress || RewardAmount < 0)
	{
		return Result;
	}

	Snapshot.Wallet += RewardAmount;
	Commit();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryGrantKey(FName AccessCategory, int32 Quantity)
{
	FCommandResult Result;

	if (Snapshot.RunStatus != ERunStatus::InProgress || Quantity <= 0)
	{
		return Result;
	}

	Snapshot.KeyCounts.FindOrAdd(AccessCategory) += Quantity;
	Commit();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryConsumeKey(FName AccessCategory)
{
	FCommandResult Result;

	if (Snapshot.RunStatus != ERunStatus::InProgress || Snapshot.KeyCounts.FindRef(AccessCategory) < 1)
	{
		Result.FailureReason = TAG_Result_Failure_MissingPrerequisite;
		return Result;
	}

	Snapshot.KeyCounts[AccessCategory] -= 1;
	CommitAndSave();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryOpenChest(FName RequiredAccessCategory, int64 RewardAmount)
{
	FCommandResult Result;

	if (Snapshot.RunStatus != ERunStatus::InProgress || RewardAmount < 0)
	{
		return Result;
	}

	const bool bKeyRequired = !RequiredAccessCategory.IsNone();
	if (bKeyRequired && Snapshot.KeyCounts.FindRef(RequiredAccessCategory) < 1)
	{
		Result.FailureReason = TAG_Result_Failure_MissingPrerequisite;
		return Result;
	}

	if (bKeyRequired)
	{
		Snapshot.KeyCounts[RequiredAccessCategory] -= 1;
	}
	Snapshot.Wallet += RewardAmount;
	CommitAndSave();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryDebitWallet(int64 Amount)
{
	FCommandResult Result;

	if (Amount <= 0 || Snapshot.Wallet < Amount)
	{
		return Result;
	}

	Snapshot.Wallet -= Amount;
	CommitAndSave();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

FCommandResult UProgressionSubsystem::TryPurchaseUpgrade(UUpgradeDefinition* Definition)
{
	FCommandResult Result;

	if (!Definition || Definition->ContentId.IsNone() || Definition->FlatPrice < 0)
	{
		Result.FailureReason = TAG_Result_Failure_ContentUnavailable;
		return Result;
	}

	RegisterUpgradeDefinition(Definition);

	const int32 CurrentRank = Snapshot.OwnedUpgradeRanks.FindRef(Definition->ContentId);
	if (CurrentRank >= Definition->MaxRank)
	{
		Result.FailureReason = TAG_Result_Failure_AlreadyOwned;
		return Result;
	}

	for (const FName& PrerequisiteId : Definition->PrerequisiteContentIds)
	{
		if (Snapshot.OwnedUpgradeRanks.FindRef(PrerequisiteId) < 1)
		{
			Result.FailureReason = TAG_Result_Failure_MissingPrerequisite;
			return Result;
		}
	}

	if (Snapshot.Wallet < Definition->FlatPrice)
	{
		Result.FailureReason = TAG_Result_Failure_InsufficientFunds;
		return Result;
	}

	Snapshot.Wallet -= Definition->FlatPrice;
	Snapshot.OwnedUpgradeRanks.FindOrAdd(Definition->ContentId) = CurrentRank + 1;
	CommitAndSave();

	Result.bSucceeded = true;
	Result.Revision = Snapshot.Revision;
	return Result;
}

void UProgressionSubsystem::RegisterUpgradeDefinition(UUpgradeDefinition* Definition)
{
	if (Definition && !Definition->ContentId.IsNone())
	{
		UpgradeDefinitionsByContentId.Add(Definition->ContentId, Definition);
	}
}

void UProgressionSubsystem::RegisterAbilityDefinition(UAbilityDefinition* Definition)
{
	if (Definition && !Definition->ContentId.IsNone())
	{
		AbilityDefinitionsByContentId.Add(Definition->ContentId, Definition);
	}
}

void UProgressionSubsystem::ReconcileAbilityGrants(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const TPair<FName, int32>& OwnedRank : Snapshot.OwnedUpgradeRanks)
	{
		if (OwnedRank.Value <= 0)
		{
			continue;
		}

		const TObjectPtr<UUpgradeDefinition>* UpgradeDefinitionPtr = UpgradeDefinitionsByContentId.Find(OwnedRank.Key);
		if (!UpgradeDefinitionPtr || !*UpgradeDefinitionPtr || !(*UpgradeDefinitionPtr)->GrantedAbilityId.IsValid())
		{
			continue;
		}

		const FName AbilityContentId = (*UpgradeDefinitionPtr)->GrantedAbilityId.GetTagName();
		const TObjectPtr<UAbilityDefinition>* AbilityDefinitionPtr = AbilityDefinitionsByContentId.Find(AbilityContentId);
		if (!AbilityDefinitionPtr || !*AbilityDefinitionPtr || !(*AbilityDefinitionPtr)->AbilityClass)
		{
			continue;
		}

		const TSubclassOf<UGameplayAbility> AbilityClass = (*AbilityDefinitionPtr)->AbilityClass;
		if (AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
		{
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, (*AbilityDefinitionPtr)->AbilityLevel, INDEX_NONE, this);
		const FGameplayTag& InputTag = (*AbilityDefinitionPtr)->InputTag;
		if (InputTag.IsValid())
		{
			// Makes UAbilityDefinition::InputTag real: ASOTMPlayerCharacter::
			// TryActivateAbilityByInputTag resolves a pressed input slot to
			// whichever ability was granted for it by searching this tag on
			// each spec, rather than either side hardcoding a specific
			// ability class (handbook Part 2 section 1's "ASC input adapter").
			Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
		}

		AbilitySystemComponent->GiveAbility(Spec);
	}
}
