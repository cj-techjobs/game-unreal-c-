// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SOTMJumpscareEnemyCharacter.generated.h"

class UAnimationAsset;
class UPrimitiveComponent;
struct FHitResult;

/**
 * A simple always-chasing enemy that jump-scares on contact rather than
 * dealing combat damage (e.g. Creation). Distinct from ASOTMEnemyCharacter,
 * whose "attack" is a ranged combat action -- this one's only interaction is
 * a one-shot camera-cut + input-disable + reaction animation when it
 * physically touches the player.
 *
 * Ported from the original Creation Blueprint, which cast the touching actor
 * to BP_ThirdPersonCharacter specifically and called a custom "Jumpscare"
 * event on it. This version works off the generic Pawn/PlayerController
 * instead, so it does not silently break if the live player class changes
 * later (the same class of bug found in BP_WEAPON during the architecture
 * audit).
 */
UCLASS()
class SOTMGAMEPLAY_API ASOTMJumpscareEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASOTMJumpscareEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	/** Single-shot reaction animation played on contact (e.g. "Creation--Slashed"). */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Jumpscare")
	TObjectPtr<UAnimationAsset> JumpscareAnim;

	/** Camera blend duration when cutting the player's view to this actor on contact. */
	UPROPERTY(EditDefaultsOnly, Category = "SOTM|Enemy|Jumpscare")
	float CameraBlendSeconds = 0.1f;

	UFUNCTION()
	void HandleTouchOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
