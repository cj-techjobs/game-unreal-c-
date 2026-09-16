// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectiveSubsystem.h"
#include "ObjectiveDefinition.h"
#include "ProgressionSubsystem.h"
#include "Engine/GameInstance.h"

bool UObjectiveSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UObjectiveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			Progression->OnProgressionChanged.AddDynamic(this, &UObjectiveSubsystem::HandleProgressionChanged);
		}
	}
}

void UObjectiveSubsystem::Deinitialize()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			Progression->OnProgressionChanged.RemoveDynamic(this, &UObjectiveSubsystem::HandleProgressionChanged);
		}
	}

	Super::Deinitialize();
}

void UObjectiveSubsystem::Transition(const FObjectiveState& State)
{
	OnObjectiveTransitioned.Broadcast(State);
}

FCommandResult UObjectiveSubsystem::ActivateObjective(FGameplayTag ObjectiveId, int32 Target)
{
	FCommandResult Result;

	if (!ObjectiveId.IsValid() || Target < 1)
	{
		return Result;
	}

	FObjectiveState& State = ObjectiveStates.FindOrAdd(ObjectiveId);
	if (State.Status == EObjectiveStatus::Active || State.Status == EObjectiveStatus::Completed)
	{
		return Result;
	}

	State.ObjectiveId = ObjectiveId;
	State.Status = EObjectiveStatus::Active;
	State.Progress = 0;
	State.Target = Target;

	Result.bSucceeded = true;
	Transition(State);
	return Result;
}

FCommandResult UObjectiveSubsystem::ActivateObjectiveFromDefinition(UObjectiveDefinition* Definition)
{
	if (!Definition)
	{
		return FCommandResult();
	}

	FCommandResult Result = ActivateObjective(Definition->ObjectiveId, Definition->ProgressTarget);
	if (Result.bSucceeded)
	{
		DefinitionsByObjectiveId.Add(Definition->ObjectiveId, Definition);
	}

	return Result;
}

UObjectiveDefinition* UObjectiveSubsystem::GetObjectiveDefinition(FGameplayTag ObjectiveId) const
{
	const TObjectPtr<UObjectiveDefinition>* Found = DefinitionsByObjectiveId.Find(ObjectiveId);
	return Found ? Found->Get() : nullptr;
}

FCommandResult UObjectiveSubsystem::AdvanceObjective(FGameplayTag ObjectiveId, int32 Amount)
{
	FCommandResult Result;

	if (Amount <= 0)
	{
		return Result;
	}

	FObjectiveState* State = ObjectiveStates.Find(ObjectiveId);
	if (!State || State->Status != EObjectiveStatus::Active)
	{
		return Result;
	}

	State->Progress = FMath::Min(State->Progress + Amount, State->Target);
	if (State->Progress >= State->Target)
	{
		State->Status = EObjectiveStatus::Completed;
	}

	Result.bSucceeded = true;
	Transition(*State);
	return Result;
}

FCommandResult UObjectiveSubsystem::CompleteObjective(FGameplayTag ObjectiveId)
{
	FCommandResult Result;

	FObjectiveState* State = ObjectiveStates.Find(ObjectiveId);
	if (!State || State->Status != EObjectiveStatus::Active)
	{
		return Result;
	}

	State->Status = EObjectiveStatus::Completed;
	State->Progress = State->Target;

	Result.bSucceeded = true;
	Transition(*State);
	return Result;
}

FCommandResult UObjectiveSubsystem::FailObjective(FGameplayTag ObjectiveId)
{
	FCommandResult Result;

	FObjectiveState* State = ObjectiveStates.Find(ObjectiveId);
	if (!State || State->Status != EObjectiveStatus::Active)
	{
		return Result;
	}

	State->Status = EObjectiveStatus::Failed;

	Result.bSucceeded = true;
	Transition(*State);
	return Result;
}

FObjectiveState UObjectiveSubsystem::GetObjectiveState(FGameplayTag ObjectiveId) const
{
	if (const FObjectiveState* State = ObjectiveStates.Find(ObjectiveId))
	{
		return *State;
	}

	FObjectiveState DefaultState;
	DefaultState.ObjectiveId = ObjectiveId;
	return DefaultState;
}

TArray<FObjectiveState> UObjectiveSubsystem::GetActiveObjectives() const
{
	TArray<FObjectiveState> ActiveObjectives;
	for (const TPair<FGameplayTag, FObjectiveState>& Pair : ObjectiveStates)
	{
		if (Pair.Value.Status == EObjectiveStatus::Active)
		{
			ActiveObjectives.Add(Pair.Value);
		}
	}
	return ActiveObjectives;
}

void UObjectiveSubsystem::ResetAllTransientState()
{
	for (TPair<FGameplayTag, FObjectiveState>& Pair : ObjectiveStates)
	{
		if (Pair.Value.Status == EObjectiveStatus::Active || Pair.Value.Status == EObjectiveStatus::Failed)
		{
			Pair.Value.Status = EObjectiveStatus::Inactive;
			Pair.Value.Progress = 0;
			Transition(Pair.Value);
		}
	}
}

void UObjectiveSubsystem::ReportAreaEntered(FGameplayTag MarkerId)
{
	if (!MarkerId.IsValid())
	{
		return;
	}

	// Collect matches before mutating: AdvanceObjective broadcasts
	// OnObjectiveTransitioned, and a listener reacting by activating another
	// objective would insert into DefinitionsByObjectiveId while this range-for
	// is still iterating it (handbook Part 1 section 7's reentrancy rule).
	TArray<FGameplayTag> MatchingObjectiveIds;
	for (const TPair<FGameplayTag, TObjectPtr<UObjectiveDefinition>>& Pair : DefinitionsByObjectiveId)
	{
		const UObjectiveDefinition* Definition = Pair.Value;
		if (Definition && Definition->ConditionType == EObjectiveConditionType::EnterArea && Definition->ConditionTargetTag == MarkerId)
		{
			MatchingObjectiveIds.Add(Pair.Key);
		}
	}

	for (const FGameplayTag& ObjectiveId : MatchingObjectiveIds)
	{
		if (GetObjectiveState(ObjectiveId).Status == EObjectiveStatus::Active)
		{
			AdvanceObjective(ObjectiveId, 1);
		}
	}
}

void UObjectiveSubsystem::HandleProgressionChanged(const FProgressionSnapshot& Snapshot)
{
	// See ReportAreaEntered: collect before mutating, for the same reentrancy reason.
	TArray<FGameplayTag> ObjectiveIdsToComplete;
	for (const TPair<FGameplayTag, TObjectPtr<UObjectiveDefinition>>& Pair : DefinitionsByObjectiveId)
	{
		const UObjectiveDefinition* Definition = Pair.Value;
		if (!Definition || GetObjectiveState(Pair.Key).Status != EObjectiveStatus::Active)
		{
			continue;
		}

		if (Definition->ConditionType == EObjectiveConditionType::HaveCurrency)
		{
			if (Snapshot.Wallet >= Definition->ProgressTarget)
			{
				ObjectiveIdsToComplete.Add(Pair.Key);
			}
		}
		else if (Definition->ConditionType == EObjectiveConditionType::OwnUpgrade)
		{
			if (Snapshot.OwnedUpgradeRanks.FindRef(Definition->ConditionTargetTag.GetTagName()) >= 1)
			{
				ObjectiveIdsToComplete.Add(Pair.Key);
			}
		}
	}

	for (const FGameplayTag& ObjectiveId : ObjectiveIdsToComplete)
	{
		CompleteObjective(ObjectiveId);
	}
}
