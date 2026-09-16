// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMChapterGameMode.h"
#include "GameFlowSubsystem.h"
#include "ChapterSubsystem.h"
#include "ProgressionSubsystem.h"
#include "SOTMProgressionTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ASOTMChapterGameMode::ASOTMChapterGameMode()
{
	// Intentionally empty for this increment.
}

void ASOTMChapterGameMode::BeginPlay()
{
	Super::BeginPlay();

	UGameFlowSubsystem* GameFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameFlowSubsystem>() : nullptr;
	if (!GameFlow)
	{
		return;
	}

	GameFlow->BeginLoading();
	GameFlow->OnTerminalCandidateResolved.AddDynamic(this, &ASOTMChapterGameMode::HandleTerminalCandidateResolved);

	UChapterSubsystem* Chapter = GetWorld() ? GetWorld()->GetSubsystem<UChapterSubsystem>() : nullptr;
	if (!Chapter)
	{
		return;
	}

	if (Chapter->IsReady())
	{
		GameFlow->EnterPlaying();
	}
	else
	{
		Chapter->OnChapterReady.AddDynamic(this, &ASOTMChapterGameMode::HandleChapterReady);
	}
}

void ASOTMChapterGameMode::HandleChapterReady()
{
	if (UGameFlowSubsystem* GameFlow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGameFlowSubsystem>() : nullptr)
	{
		GameFlow->EnterPlaying();
	}
}

void ASOTMChapterGameMode::HandleTerminalCandidateResolved(FGuid CandidateId, bool bAccepted, ETerminalCandidateKind Kind)
{
	if (!bAccepted || Kind != ETerminalCandidateKind::PlayerDeath)
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	UProgressionSubsystem* Progression = GameInstance ? GameInstance->GetSubsystem<UProgressionSubsystem>() : nullptr;
	if (!Progression || Progression->GetSnapshot().RunStatus != ERunStatus::InProgress)
	{
		// No lives left (or no subsystem at all): a real GameOver, not a retry. Presentation for that state has no owner yet -- see class comment.
		return;
	}

	UChapterSubsystem* Chapter = GetWorld() ? GetWorld()->GetSubsystem<UChapterSubsystem>() : nullptr;
	if (Chapter)
	{
		Chapter->RestoreFromLatestSave();
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}

	RestartPlayer(PlayerController);
}
