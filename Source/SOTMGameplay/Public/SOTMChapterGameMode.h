// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SOTMGameModeBase.h"
#include "GameFlowSubsystem.h"
#include "SOTMChapterGameMode.generated.h"

/**
 * Chapter rules, readiness coordination, and spawn authorization.
 * Delegates persistence and transactions to their owning subsystems.
 *
 * BeginPlay drives UGameFlowSubsystem Loading->Playing against
 * UChapterSubsystem::IsReady() -- without this, nothing in the project
 * calls BeginLoading()/EnterPlaying() at all, so the flow state permanently
 * sits at its Boot default and anything gated on "must be Playing"
 * (e.g. the terminal-resolution phase) silently never accepts. Deliberately
 * does not call ActivateChapter/ActivateChapterFromDefinition itself --
 * which chapter is active is a separate, content-driven concern (handbook
 * Part 1 section 5's level-travel step) this class doesn't invent an answer
 * for; IsReady() and IsChapterActive() are independent flags on
 * UChapterSubsystem, so readiness can (and today always does, since nothing
 * yet calls AddReadinessRequirement) resolve true with no chapter active.
 *
 * Also binds UGameFlowSubsystem::OnTerminalCandidateResolved to drive the
 * mechanical half of "die -> retry": this was the documented gap in
 * ASOTMPlayerCharacter's own class comment ("nothing yet consumes
 * OnTerminalCandidateResolved to drive respawn"). On an accepted
 * PlayerDeath candidate, if UProgressionSubsystem::GetSnapshot().RunStatus
 * is still InProgress (a life remained after GameFlowSubsystem's own
 * internal TryAcceptDeath debit), this restores the latest save and
 * replaces the pawn -- the actual GameOver-screen presentation for a
 * RunStatus that instead comes back GameOver is explicitly left alone, per
 * the same gap note ("presentation-layer work with no owner yet").
 * ChapterVictory candidates are filtered out by Kind, not left to an
 * accepted-death assumption, since accepting a victory does not mean a
 * life was consumed or a retry is wanted.
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMChapterGameMode : public ASOTMGameModeBase
{
	GENERATED_BODY()

public:
	ASOTMChapterGameMode();

	virtual void BeginPlay() override;

private:
	/** Bound only when IsReady() was false at BeginPlay -- currently dead code in practice (nothing calls AddReadinessRequirement yet) but forward-compatible for when something does. */
	UFUNCTION()
	void HandleChapterReady();

	/** See class comment. Unpossesses/destroys the current pawn and calls RestartPlayer -- AGameModeBase's own stock respawn path -- so PossessedBy re-runs ASOTMPlayerCharacter::InitializeAbilityActorInfo on the fresh avatar exactly as it does on first spawn. */
	UFUNCTION()
	void HandleTerminalCandidateResolved(FGuid CandidateId, bool bAccepted, ETerminalCandidateKind Kind);
};
