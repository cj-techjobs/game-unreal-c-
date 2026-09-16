// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SOTMAttackPlayer.h"
#include "AIController.h"
#include "SOTMEnemyCharacter.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SOTMAttackPlayer::UBTTask_SOTMAttackPlayer()
{
	NodeName = TEXT("SOTM Attack Player");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_SOTMAttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ASOTMEnemyCharacter* Enemy = AIController ? Cast<ASOTMEnemyCharacter>(AIController->GetPawn()) : nullptr;
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	Enemy->AttackPlayer();
	ElapsedSeconds = 0.0f;

	return EBTNodeResult::InProgress;
}

void UBTTask_SOTMAttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ElapsedSeconds += DeltaSeconds;
	if (ElapsedSeconds >= RecoveryDelaySeconds)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
