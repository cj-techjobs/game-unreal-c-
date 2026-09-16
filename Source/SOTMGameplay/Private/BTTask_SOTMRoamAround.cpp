// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SOTMRoamAround.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SOTMRoamAround::UBTTask_SOTMRoamAround()
{
	NodeName = TEXT("SOTM Roam Around");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SOTMRoamAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIController || !ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	FVector RandomLocation;
	const bool bFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(
		ControlledPawn, ControlledPawn->GetActorLocation(), RandomLocation, RoamRadius);

	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	AIController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_SOTMRoamAround::HandleMoveCompleted);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(RandomLocation);
	MoveRequest.SetAcceptanceRadius(50.0f);
	AIController->MoveTo(MoveRequest);

	return EBTNodeResult::InProgress;
}

void UBTTask_SOTMRoamAround::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	UBehaviorTreeComponent* OwnerComp = CachedOwnerComp.Get();
	if (!OwnerComp)
	{
		return;
	}

	if (AAIController* AIController = OwnerComp->GetAIOwner())
	{
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_SOTMRoamAround::HandleMoveCompleted);
	}

	FinishLatentTask(*OwnerComp, Result == EPathFollowingResult::Success ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
}
