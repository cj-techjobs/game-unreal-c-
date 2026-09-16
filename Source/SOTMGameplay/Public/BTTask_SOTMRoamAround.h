// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_SOTMRoamAround.generated.h"

/**
 * Moves the controlled pawn to a random reachable point within a radius
 * of its current location. Ported from the original BT_Task_RoamAround
 * Blueprint task.
 */
UCLASS()
class SOTMGAMEPLAY_API UBTTask_SOTMRoamAround : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SOTMRoamAround();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "SOTM|AI")
	float RoamRadius = 3000.0f;

	UFUNCTION()
	void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
