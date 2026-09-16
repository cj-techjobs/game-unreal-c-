// Fill out your copyright notice in the Description page of Project Settings.

#include "PresentationSubsystem.h"
#include "PlayerHUDPresenter.h"

void UPresentationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	HUDPresenter = NewObject<UPlayerHUDPresenter>(this);
	HUDPresenter->InitializeForLocalPlayer(GetLocalPlayer());
}

void UPresentationSubsystem::Deinitialize()
{
	if (HUDPresenter)
	{
		HUDPresenter->Shutdown();
	}
	HUDPresenter = nullptr;

	Super::Deinitialize();
}
