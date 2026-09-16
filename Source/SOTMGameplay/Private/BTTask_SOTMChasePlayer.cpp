// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SOTMChasePlayer.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SOTMChasePlayer::UBTTask_SOTMChasePlayer()
{
	NodeName = TEXT("SOTM Chase Player");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SOTMChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!AIController || !PlayerCharacter)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	AIController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_SOTMChasePlayer::HandleMoveCompleted);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(PlayerCharacter);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	AIController->MoveTo(MoveRequest);

	return EBTNodeResult::InProgress;
}

void UBTTask_SOTMChasePlayer::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp)
	{
		return;
	}

	if (AAIController* AIController = OwnerComp->GetAIOwner())
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_SOTMChasePlayer::HandleMoveCompleted);
	}

	FinishLatentTask(*OwnerComp, Result == EPathFollowingResult::Success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}
