// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioNarrativeSubsystem.h"

bool UAudioNarrativeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FCommandResult UAudioNarrativeSubsystem::RequestDialogueCue(FGameplayTag CueId, FGameplayTag SpeakerId, EDialoguePriority Priority)
{
	FCommandResult Result;

	if (!CueId.IsValid())
	{
		return Result;
	}

	FDialogueCueRequest NewCue;
	NewCue.CueId = CueId;
	NewCue.SpeakerId = SpeakerId;
	NewCue.Priority = Priority;

	if (!IsDialoguePlaying() || Priority >= CurrentCue.Priority)
	{
		CurrentCue = NewCue;
		OnDialogueCueChanged.Broadcast(CurrentCue);
	}
	else
	{
		DialogueQueue.Add(NewCue);
		DialogueQueue.Sort([](const FDialogueCueRequest& A, const FDialogueCueRequest& B)
		{
			return A.Priority > B.Priority;
		});
	}

	Result.bSucceeded = true;
	return Result;
}

void UAudioNarrativeSubsystem::CompleteDialogueCue(FGameplayTag CueId)
{
	if (CurrentCue.CueId != CueId)
	{
		return;
	}

	AdvanceDialogueQueue();
}

void UAudioNarrativeSubsystem::ClearDialogueQueue()
{
	DialogueQueue.Empty();
	CurrentCue = FDialogueCueRequest();
	OnDialogueCueChanged.Broadcast(CurrentCue);
}

void UAudioNarrativeSubsystem::AdvanceDialogueQueue()
{
	if (DialogueQueue.Num() > 0)
	{
		CurrentCue = DialogueQueue[0];
		DialogueQueue.RemoveAt(0);
	}
	else
	{
		CurrentCue = FDialogueCueRequest();
	}

	OnDialogueCueChanged.Broadcast(CurrentCue);
}

void UAudioNarrativeSubsystem::SubmitMusicIntent(EMusicIntent Intent)
{
	SubmittedMusicIntents.Add(Intent);
	RecomputeActiveMusicIntent();
}

void UAudioNarrativeSubsystem::ClearMusicIntent(EMusicIntent Intent)
{
	SubmittedMusicIntents.Remove(Intent);
	RecomputeActiveMusicIntent();
}

void UAudioNarrativeSubsystem::RecomputeActiveMusicIntent()
{
	EMusicIntent Highest = EMusicIntent::Exploration;
	bool bFoundAny = false;

	for (EMusicIntent Intent : SubmittedMusicIntents)
	{
		if (!bFoundAny || Intent > Highest)
		{
			Highest = Intent;
			bFoundAny = true;
		}
	}

	if (ActiveMusicIntent != Highest)
	{
		ActiveMusicIntent = Highest;
		OnMusicIntentChanged.Broadcast(ActiveMusicIntent);
	}
}
