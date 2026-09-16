// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_SOTMChasePlayer.generated.h"

/**
 * Moves the controlled pawn directly toward the player character.
 * Ported from the original BT_Task_ChasePlayer Blueprint task.
 */
UCLASS()
class SOTMGAMEPLAY_API UBTTask_SOTMChasePlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SOTMChasePlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "SOTM|AI")
	float AcceptanceRadius = 120.0f;

	UFUNCTION()
	void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
