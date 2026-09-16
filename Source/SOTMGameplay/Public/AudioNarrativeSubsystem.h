// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SOTMNarrativeTypes.h"
#include "SOTMCommandTypes.h"
#include "AudioNarrativeSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCueChanged, const FDialogueCueRequest&, Cue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicIntentChanged, EMusicIntent, Intent);

/**
 * Dialogue queue and music intent selection. Gameplay-only: excluded from
 * editor preview worlds so it never activates outside Game/PIE.
 *
 * Deliberately covers only the queue/priority mechanics the handbook's
 * Part 2 section 9/9.1 describes, not the content: no voice playback,
 * subtitle display, dialogue-row data table, or hint-eligibility
 * evaluation against objective markers (ISOTMObjectiveTarget actors don't
 * exist yet to resolve hint targets from). A presentation layer is
 * expected to listen to OnDialogueCueChanged/OnMusicIntentChanged and
 * actually play something.
 */
UCLASS()
class SOTMGAMEPLAY_API UAudioNarrativeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Narrative")
	FOnDialogueCueChanged OnDialogueCueChanged;

	UPROPERTY(BlueprintAssignable, Category = "SOTM|Narrative")
	FOnMusicIntentChanged OnMusicIntentChanged;

	/** Interrupts immediately if nothing is playing or Priority is at least as high as the current cue's; otherwise queues it (highest-priority-first). Fails only if CueId is invalid. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Narrative")
	FCommandResult RequestDialogueCue(FGameplayTag CueId, FGameplayTag SpeakerId, EDialoguePriority Priority);

	/** Called by the presentation layer once a cue's line finishes; advances to the next queued cue if CueId matches what's currently playing (a stale/duplicate completion for an already-superseded cue is ignored). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Narrative")
	void CompleteDialogueCue(FGameplayTag CueId);

	/** Clears the queue and current cue, e.g. on death/travel. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Narrative")
	void ClearDialogueQueue();

	UFUNCTION(BlueprintPure, Category = "SOTM|Narrative")
	FDialogueCueRequest GetCurrentDialogueCue() const { return CurrentCue; }

	UFUNCTION(BlueprintPure, Category = "SOTM|Narrative")
	bool IsDialoguePlaying() const { return CurrentCue.CueId.IsValid(); }

	/** Adds Intent to the submitted set; the active intent becomes the highest-priority one currently submitted (multiple AI controllers can each submit Chase without racing to "own" the track). */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Narrative")
	void SubmitMusicIntent(EMusicIntent Intent);

	/** Removes Intent from the submitted set; the active intent falls back to the next-highest still-submitted one, or Exploration if none remain. */
	UFUNCTION(BlueprintCallable, Category = "SOTM|Narrative")
	void ClearMusicIntent(EMusicIntent Intent);

	UFUNCTION(BlueprintPure, Category = "SOTM|Narrative")
	EMusicIntent GetActiveMusicIntent() const { return ActiveMusicIntent; }

private:
	UPROPERTY()
	TArray<FDialogueCueRequest> DialogueQueue;

	UPROPERTY()
	FDialogueCueRequest CurrentCue;

	UPROPERTY()
	TSet<EMusicIntent> SubmittedMusicIntents;

	UPROPERTY()
	EMusicIntent ActiveMusicIntent = EMusicIntent::Exploration;

	void AdvanceDialogueQueue();
	void RecomputeActiveMusicIntent();
};
