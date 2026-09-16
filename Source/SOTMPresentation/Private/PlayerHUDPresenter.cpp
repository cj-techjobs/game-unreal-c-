// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerHUDPresenter.h"
#include "ProgressionSubsystem.h"
#include "ObjectiveSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UPlayerHUDPresenter::InitializeForLocalPlayer(ULocalPlayer* InLocalPlayer)
{
	OwningLocalPlayer = InLocalPlayer;

	if (UGameInstance* GameInstance = InLocalPlayer ? InLocalPlayer->GetGameInstance() : nullptr)
	{
		if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			Progression->OnProgressionChanged.AddDynamic(this, &UPlayerHUDPresenter::HandleProgressionChanged);
		}
	}

	RefreshSnapshot();
}

void UPlayerHUDPresenter::Shutdown()
{
	if (ULocalPlayer* LocalPlayer = OwningLocalPlayer.Get())
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
			{
				Progression->OnProgressionChanged.RemoveDynamic(this, &UPlayerHUDPresenter::HandleProgressionChanged);
			}
		}
	}

	if (UObjectiveSubsystem* Objective = BoundObjectiveSubsystem.Get())
	{
		Objective->OnObjectiveTransitioned.RemoveDynamic(this, &UPlayerHUDPresenter::HandleObjectiveTransitioned);
	}

	BoundObjectiveSubsystem = nullptr;
	OwningLocalPlayer = nullptr;
}

UObjectiveSubsystem* UPlayerHUDPresenter::ResolveObjectiveSubsystem() const
{
	ULocalPlayer* LocalPlayer = OwningLocalPlayer.Get();
	UGameInstance* GameInstance = LocalPlayer ? LocalPlayer->GetGameInstance() : nullptr;
	UWorld* World = GameInstance ? GameInstance->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UObjectiveSubsystem>() : nullptr;
}

void UPlayerHUDPresenter::EnsureObjectiveBinding()
{
	UObjectiveSubsystem* Current = ResolveObjectiveSubsystem();
	UObjectiveSubsystem* Bound = BoundObjectiveSubsystem.Get();

	if (Current == Bound)
	{
		return;
	}

	if (Bound)
	{
		Bound->OnObjectiveTransitioned.RemoveDynamic(this, &UPlayerHUDPresenter::HandleObjectiveTransitioned);
	}

	if (Current)
	{
		Current->OnObjectiveTransitioned.AddDynamic(this, &UPlayerHUDPresenter::HandleObjectiveTransitioned);
	}

	BoundObjectiveSubsystem = Current;
}

void UPlayerHUDPresenter::RefreshSnapshot()
{
	EnsureObjectiveBinding();

	if (ULocalPlayer* LocalPlayer = OwningLocalPlayer.Get())
	{
		if (UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UProgressionSubsystem* Progression = GameInstance->GetSubsystem<UProgressionSubsystem>())
			{
				CachedProgression = Progression->GetSnapshot();
			}
		}
	}

	if (UObjectiveSubsystem* Objective = BoundObjectiveSubsystem.Get())
	{
		CachedObjectives = Objective->GetActiveObjectives();
	}
	else
	{
		CachedObjectives.Reset();
	}

	OnHUDSnapshotChanged.Broadcast();
}

void UPlayerHUDPresenter::HandleProgressionChanged(const FProgressionSnapshot& Snapshot)
{
	CachedProgression = Snapshot;
	OnHUDSnapshotChanged.Broadcast();
}

void UPlayerHUDPresenter::HandleObjectiveTransitioned(const FObjectiveState& State)
{
	RefreshSnapshot();
}
