// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PresentationSubsystem.generated.h"

class UPlayerHUDPresenter;

/**
 * Screen layers, presenters, local restriction leases, and camera/UI
 * input policy for one local player (handbook Part 2 section 13).
 *
 * Only the presenter half exists so far: UPlayerHUDPresenter, created and
 * torn down with this subsystem. The CommonUI screen-layer stack
 * (HUD/contextual/modal/cinematic/loading layers) and the restriction-
 * lease/camera-input router are M6 scope (handbook Part 4 section 1) and
 * need actual widget classes that don't exist yet.
 */
UCLASS()
class SOTMPRESENTATION_API UPresentationSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "SOTM|Presentation")
	UPlayerHUDPresenter* GetHUDPresenter() const { return HUDPresenter; }

private:
	UPROPERTY()
	TObjectPtr<UPlayerHUDPresenter> HUDPresenter;
};
