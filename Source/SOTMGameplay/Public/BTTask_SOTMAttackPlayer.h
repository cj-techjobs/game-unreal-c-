// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SOTMAttackPlayer.generated.h"

/**
 * Triggers the controlled pawn's attack and waits for its recovery delay
 * before finishing. Ported from the original BT_Task_AttackPlayer
 * Blueprint task, which cast the pawn to BP_AI and called its
 * AttackPlayer event.
 */
UCLASS()
class SOTMGAMEPLAY_API UBTTask_SOTMAttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SOTMAttackPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	/** Recovery delay after triggering the attack, before this task reports success. */
	UPROPERTY(EditAnywhere, Category = "SOTM|AI")
	float RecoveryDelaySeconds = 1.0f;

private:
	float ElapsedSeconds = 0.0f;
};
