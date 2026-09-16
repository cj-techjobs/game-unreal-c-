// Fill out your copyright notice in the Description page of Project Settings.

#include "ChapterSubsystem.h"
#include "ChapterDefinition.h"
#include "SaveSubsystem.h"
#include "WorldStateSubsystem.h"
#include "ObjectiveSubsystem.h"
#include "ProgressionSubsystem.h"
#include "Engine/GameInstance.h"

bool UChapterSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UChapterSubsystem::ActivateChapter(FGameplayTag ChapterId)
{
	ActiveChapterId = ChapterId;
	OutstandingRequirements.Empty();
	bReadyBroadcast = false;
}

void UChapterSubsystem::ActivateChapterFromDefinition(UChapterDefinition* Definition)
{
	if (!Definition)
	{
		return;
	}

	ActiveChapterDefinition = Definition;
	ActivateChapter(Definition->ChapterId);

	if (UObjectiveSubsystem* Objectives = GetWorld()->GetSubsystem<UObjectiveSubsystem>())
	{
		for (const FGameplayTag& ObjectiveId : Definition->ObjectiveRootIds)
		{
			Objectives->ActivateObjective(ObjectiveId);
		}
	}
}

void UChapterSubsystem::DeactivateChapter()
{
	ActiveChapterId = FGameplayTag();
	ActiveChapterDefinition = nullptr;
	OutstandingRequirements.Empty();
	bReadyBroadcast = false;
}

void UChapterSubsystem::AddReadinessRequirement(FGameplayTag RequirementId)
{
	if (RequirementId.IsValid())
	{
		OutstandingRequirements.Add(RequirementId);
		bReadyBroadcast = false;
	}
}

void UChapterSubsystem::SatisfyReadinessRequirement(FGameplayTag RequirementId)
{
	OutstandingRequirements.Remove(RequirementId);

	if (OutstandingRequirements.IsEmpty() && !bReadyBroadcast)
	{
		bReadyBroadcast = true;
		OnChapterReady.Broadcast();
	}
}

TArray<FGameplayTag> UChapterSubsystem::GetOutstandingRequirements() const
{
	return OutstandingRequirements.Array();
}

FCommandResult UChapterSubsystem::RestoreFromLatestSave()
{
	FCommandResult Result;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	USaveSubsystem* SaveSys = GameInstance ? GameInstance->GetSubsystem<USaveSubsystem>() : nullptr;
	if (!SaveSys)
	{
		return Result;
	}

	USOTMSaveGame* Loaded = SaveSys->LoadLatestValid();
	if (!Loaded)
	{
		return Result;
	}

	if (UWorldStateSubsystem* WorldState = GetWorld()->GetSubsystem<UWorldStateSubsystem>())
	{
		WorldState->ApplyRetainedRecords(Loaded->RetainedWorldRecords);
	}

	if (UObjectiveSubsystem* Objectives = GetWorld()->GetSubsystem<UObjectiveSubsystem>())
	{
		Objectives->ResetAllTransientState();
	}

	// Restored last, after the objective reset above: RestoreSnapshot's Commit() broadcasts OnProgressionChanged, which UObjectiveSubsystem reactively re-evaluates HaveCurrency/OwnUpgrade conditions against -- doing this before the reset would have that re-evaluation immediately clobbered by ResetAllTransientState() right after.
	if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
	{
		Progression->RestoreSnapshot(Loaded->Progression);
	}

	Result.bSucceeded = true;
	Result.Revision = Loaded->Header.SnapshotRevision;
	return Result;
}
