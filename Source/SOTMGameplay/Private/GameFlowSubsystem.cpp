// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFlowSubsystem.h"
#include "ProgressionSubsystem.h"
#include "Engine/GameInstance.h"

void UGameFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SetTickableTickType(ETickableTickType::Always);
}

void UGameFlowSubsystem::Deinitialize()
{
	SetTickableTickType(ETickableTickType::Never);

	Super::Deinitialize();
}

void UGameFlowSubsystem::SetState(EGameFlowState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnGameFlowStateChanged.Broadcast(CurrentState);
	}
}

FCommandResult UGameFlowSubsystem::BeginLoading()
{
	FCommandResult Result;

	if (CurrentState == EGameFlowState::ShuttingDown)
	{
		return Result;
	}

	CurrentGeneration = CurrentGeneration.Next();
	SetState(EGameFlowState::Loading);

	Result.bSucceeded = true;
	return Result;
}

FCommandResult UGameFlowSubsystem::EnterPlaying()
{
	FCommandResult Result;

	if (CurrentState != EGameFlowState::Loading)
	{
		return Result;
	}

	SetState(EGameFlowState::Playing);

	Result.bSucceeded = true;
	return Result;
}

void UGameFlowSubsystem::EnterFrontend()
{
	if (CurrentState != EGameFlowState::ShuttingDown)
	{
		SetState(EGameFlowState::Frontend);
	}
}

void UGameFlowSubsystem::BeginShutdown()
{
	SetState(EGameFlowState::ShuttingDown);
}

FGuid UGameFlowSubsystem::TryAcceptDeath(const FGuid& DeathEventId)
{
	if (!DeathEventId.IsValid() || SeenDeathEventIds.Contains(DeathEventId) || bTerminalFlowAccepted || CurrentState != EGameFlowState::Playing)
	{
		return FGuid();
	}

	SeenDeathEventIds.Add(DeathEventId);

	FSOTMTerminalCandidate& Candidate = PendingCandidates.AddDefaulted_GetRef();
	Candidate.CandidateId = FGuid::NewGuid();
	Candidate.Kind = ETerminalCandidateKind::PlayerDeath;
	return Candidate.CandidateId;
}

FGuid UGameFlowSubsystem::TryAcceptVictory()
{
	if (bTerminalFlowAccepted || CurrentState != EGameFlowState::Playing)
	{
		return FGuid();
	}

	FSOTMTerminalCandidate& Candidate = PendingCandidates.AddDefaulted_GetRef();
	Candidate.CandidateId = FGuid::NewGuid();
	Candidate.Kind = ETerminalCandidateKind::ChapterVictory;
	return Candidate.CandidateId;
}

void UGameFlowSubsystem::Tick(float DeltaTime)
{
	ResolvePendingCandidates();
}

void UGameFlowSubsystem::ResolvePendingCandidates()
{
	if (PendingCandidates.Num() == 0)
	{
		return;
	}

	const TArray<FSOTMTerminalCandidate> Batch = MoveTemp(PendingCandidates);

	if (bTerminalFlowAccepted)
	{
		// An already-accepted terminal flow rejects later stale candidates.
		for (const FSOTMTerminalCandidate& Candidate : Batch)
		{
			OnTerminalCandidateResolved.Broadcast(Candidate.CandidateId, false, Candidate.Kind);
		}
		return;
	}

	// A chapter-victory candidate takes priority over a simultaneous player-death candidate; otherwise accept the first-queued player death.
	int32 WinningIndex = Batch.IndexOfByPredicate([](const FSOTMTerminalCandidate& Candidate)
	{
		return Candidate.Kind == ETerminalCandidateKind::ChapterVictory;
	});
	if (WinningIndex == INDEX_NONE)
	{
		WinningIndex = 0;
	}

	bool bAccepted = false;
	if (Batch[WinningIndex].Kind == ETerminalCandidateKind::ChapterVictory)
	{
		// No UChapterSubsystem completion contract to validate/commit against yet -- see class comment.
		bAccepted = true;
	}
	else if (UProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<UProgressionSubsystem>() : nullptr)
	{
		bAccepted = Progression->TryAcceptDeath().bSucceeded;
	}

	bTerminalFlowAccepted = bAccepted;

	for (int32 Index = 0; Index < Batch.Num(); ++Index)
	{
		OnTerminalCandidateResolved.Broadcast(Batch[Index].CandidateId, bAccepted && Index == WinningIndex, Batch[Index].Kind);
	}
}
