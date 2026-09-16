// Fill out your copyright notice in the Description page of Project Settings.

#include "SOTMAIController.h"
#include "SOTMPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "SOTMStatusTags.h"
#include "TimerManager.h"
#include "SOTMGameplay.h"

const FName ASOTMAIController::SeeingPlayerKeyName(TEXT("seeingPlayer"));

ASOTMAIController::ASOTMAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	SetPerceptionComponent(*Perception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	// Implementation is a `config` UPROPERTY normally resolved from Engine/Config/BaseGame.ini; set
	// explicitly rather than trust that ini inheritance, since it silently failed to resolve for
	// Hearing in this project (see the PostInitProperties override below for the rest of the fix).
	SightConfig->Implementation = UAISense_Sight::StaticClass();
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1600.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	// All three detection flags default to false; without setting them the
	// AI would never perceive anyone regardless of team/affiliation setup.
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->SetMaxAge(0.5f);

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->Implementation = UAISense_Hearing::StaticClass();
	HearingConfig->HearingRange = 2000.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->SetMaxAge(2.0f);

	Perception->ConfigureSense(*SightConfig);
	Perception->ConfigureSense(*HearingConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
	Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ASOTMAIController::HandleTargetPerceptionUpdated);
}

void ASOTMAIController::PostInitProperties()
{
	Super::PostInitProperties();

	// Re-assert here, not just in the constructor: a Blueprint child's own compiled CDO (BP_AI_Controller)
	// can retain a stale/None copy of a config-loaded subobject property independently of whatever the
	// native constructor just set. PostInitProperties runs after all archetype/config propagation for
	// every instance, native CDO and Blueprint CDO alike, so it is the one place guaranteed the last word.
	if (SightConfig)
	{
		SightConfig->Implementation = UAISense_Sight::StaticClass();
	}
	if (HearingConfig)
	{
		HearingConfig->Implementation = UAISense_Hearing::StaticClass();
	}
}

void ASOTMAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UAIPerceptionComponent* Perception = GetAIPerceptionComponent())
	{
		// Re-assert both configs here, not just in the constructor: on a Blueprint child (BP_AI_Controller)
		// the PerceptionComponent's own SensesConfig array can carry stale/mismatched entries serialized
		// from before Hearing was ever added on the native side, and recompiling the Blueprint does not
		// regenerate that array from the current constructor. ConfigureSense is safe to call repeatedly
		// (it replaces-in-place if a config of that class already exists) and, called here -- after the
		// component is world-registered -- takes the eager registration path immediately rather than
		// waiting on OnRegister(), which is what silently never picked up Hearing before this was added.
		Perception->ConfigureSense(*SightConfig);
		Perception->ConfigureSense(*HearingConfig);

		// ConfigureSense's eager path registers the sense class globally and updates this component's
		// own PerceptionFilter, but the AIPerceptionSystem's FPerceptionListener entry for this component
		// was already created back at OnRegister() time (Sight-only) and caches its own copy of that
		// filter -- UAISense_Hearing::OnListenerUpdateImpl checks the cached copy, not the component's
		// live PerceptionFilter, to decide whether to build this listener's per-sense digested properties.
		// Without refreshing it, Hearing's sense ID resolves valid and NoiseEvents get reported, but this
		// listener is silently skipped in UAISense_Hearing::Update() forever. UpdateListener() is the same
		// call OnRegister() itself makes after processing SensesConfig; calling it again re-syncs the cache.
		if (UAIPerceptionSystem* PerceptionSys = UAIPerceptionSystem::GetCurrent(GetWorld()))
		{
			PerceptionSys->UpdateListener(*Perception);
		}
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsBool(SeeingPlayerKeyName, false);
	}

	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(InPawn);
	UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (ASC)
	{
		StunTagEventHandle = ASC->RegisterGameplayTagEvent(TAG_State_Stunned, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ASOTMAIController::HandleStunTagChanged);
	}
}

void ASOTMAIController::OnUnPossess()
{
	const IAbilitySystemInterface* ASI = GetPawn() ? Cast<IAbilitySystemInterface>(GetPawn()) : nullptr;
	UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (ASC && StunTagEventHandle.IsValid())
	{
		ASC->UnregisterGameplayTagEvent(StunTagEventHandle, TAG_State_Stunned, EGameplayTagEventType::NewOrRemoved);
		StunTagEventHandle.Reset();
	}

	Super::OnUnPossess();
}

void ASOTMAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bAlwaysChasePlayer)
	{
		if (ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0))
		{
			MoveToActor(PlayerCharacter, AlwaysChaseAcceptanceRadius, false);
		}
	}

	// AIPerceptionComponent only re-fires OnTargetPerceptionUpdated on a sense/state CHANGE, not every tick a
	// target remains steadily sensed -- so decay must be gated on CurrentVisibleTarget directly (a live fact,
	// cleared the instant sight is lost) rather than trusting the delegate to keep re-raising Suspicion while
	// still visible; the single "newly seen" edge never repeats on its own.
	if (!CurrentVisibleTarget.IsValid() && Suspicion > 0.0f)
	{
		Suspicion = FMath::Max(0.0f, Suspicion - SuspicionDecayPerSecond * DeltaSeconds);
	}
}

void ASOTMAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Actor->IsA<ASOTMPlayerCharacter>())
	{
		return;
	}

	const bool bSensed = Stimulus.WasSuccessfullySensed();
	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.0f;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (bSensed)
		{
			CurrentVisibleTarget = Actor;
			LastSeenLocation = Stimulus.StimulusLocation;
			LastSeenTime = Now;
			Suspicion = 1.0f;

			// Sight re-acquired: Chase should win over whatever Investigate/Search was doing, immediately.
			EndInvestigate();

			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsBool(SeeingPlayerKeyName, true);
			}
		}
		else
		{
			CurrentVisibleTarget = nullptr;

			if (UBlackboardComponent* BB = GetBlackboardComponent())
			{
				BB->SetValueAsBool(SeeingPlayerKeyName, false);
			}

			// "On loss of sight, move toward last confirmed position, search a bounded area, then return" (handbook Part 2 section 4.1).
			if (LastSeenTime >= 0.0f)
			{
				BeginInvestigate(LastSeenLocation);
			}
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (bSensed)
		{
			LastHeardLocation = Stimulus.StimulusLocation;
			LastHeardTime = Now;
			// Hearing is a weaker signal than sight (handbook: "it does not reveal current player position indefinitely") -- nudge, don't pin to max.
			Suspicion = FMath::Min(1.0f, Suspicion + 0.5f);

			// Only worth investigating a noise if we're not already looking straight at the source.
			if (!CurrentVisibleTarget.IsValid())
			{
				BeginInvestigate(LastHeardLocation);
			}
		}
	}
}

void ASOTMAIController::HandleStunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// See class comment: PauseLogic/ResumeLogic stands in for a real Stunned
	// priority branch, since no BT-graph-editing tool exists here to add one
	// to BT_AI. StopMovement() halts any latent MoveTo (e.g. mid-chase) that
	// PauseLogic alone would leave running, since PathFollowingComponent is
	// driven independently of the Behavior Tree's own tick.
	if (NewCount > 0)
	{
		StopMovement();
		if (UBrainComponent* Brain = GetBrainComponent())
		{
			Brain->PauseLogic(TEXT("Stunned"));
		}
	}
	// Only hand control back if Investigate/Search isn't still holding its own pause -- otherwise a stun that
	// resolves mid-search would let the Behavior Tree and the pending MoveTo fight over movement commands.
	else if (CurrentSearchPhase == EAISearchPhase::None)
	{
		if (UBrainComponent* Brain = GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("Stunned"));
		}
	}
}

bool ASOTMAIController::IsCurrentlyStunned() const
{
	const IAbilitySystemInterface* ASI = GetPawn() ? Cast<IAbilitySystemInterface>(GetPawn()) : nullptr;
	const UAbilitySystemComponent* ASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	return ASC && ASC->HasMatchingGameplayTag(TAG_State_Stunned);
}

void ASOTMAIController::BeginInvestigate(const FVector& Location)
{
	// Stun already owns the pause; moving now would contradict "stunned", and HandleStunTagChanged
	// will not have a search phase to resume into once it clears. Simplest safe answer: skip it.
	if (IsCurrentlyStunned())
	{
		return;
	}

	if (CurrentSearchPhase != EAISearchPhase::None)
	{
		// Already investigating -- redirect to the newer location rather than stacking sequences.
		GetWorldTimerManager().ClearTimer(SearchWaitTimerHandle);
		ReceiveMoveCompleted.RemoveDynamic(this, &ASOTMAIController::HandleInvestigateMoveCompleted);
	}
	else if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->PauseLogic(TEXT("Investigating"));
	}

	CurrentSearchPhase = EAISearchPhase::MovingToLocation;
	ReceiveMoveCompleted.AddDynamic(this, &ASOTMAIController::HandleInvestigateMoveCompleted);

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalLocation(Location);
	MoveRequest.SetAcceptanceRadius(InvestigateAcceptanceRadius);
	const FPathFollowingRequestResult MoveResult = MoveTo(MoveRequest);
	UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: BeginInvestigate MoveTo(%s) result=%d (0=Failed,1=AlreadyAtGoal,2=RequestSuccessful)"),
		*GetNameSafe(this), *Location.ToString(), static_cast<int32>(MoveResult.Code.GetValue()));
}

void ASOTMAIController::HandleInvestigateMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	ReceiveMoveCompleted.RemoveDynamic(this, &ASOTMAIController::HandleInvestigateMoveCompleted);
	UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: HandleInvestigateMoveCompleted Result=%d (0=Success,1=Blocked,2=OffPath,3=Aborted,4=Skipped_UNUSED,5=Invalid)"),
		*GetNameSafe(this), static_cast<int32>(Result));

	if (CurrentSearchPhase != EAISearchPhase::MovingToLocation)
	{
		return;
	}

	// Arrived, blocked, or gave up -- any outcome still means "search here for a beat" per the handbook's
	// "search a bounded area" step; a bounded-area sweep (EQS or a few nearby points) is the natural
	// upgrade once this baseline is proven, not required for a first vertical slice (handbook Part 2
	// section 4.3: "EQS is appropriate... after the baseline works... not required for every patrol waypoint").
	CurrentSearchPhase = EAISearchPhase::Waiting;
	GetWorldTimerManager().SetTimer(SearchWaitTimerHandle, this, &ASOTMAIController::HandleSearchWaitComplete, SearchWaitSeconds, false);
}

void ASOTMAIController::HandleSearchWaitComplete()
{
	EndInvestigate();
}

void ASOTMAIController::EndInvestigate()
{
	if (CurrentSearchPhase == EAISearchPhase::None)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(SearchWaitTimerHandle);
	ReceiveMoveCompleted.RemoveDynamic(this, &ASOTMAIController::HandleInvestigateMoveCompleted);
	CurrentSearchPhase = EAISearchPhase::None;
	UE_LOG(LogSOTMGameplay, Verbose, TEXT("%s: EndInvestigate"), *GetNameSafe(this));

	// A still-active stun keeps owning the pause; it will resume logic itself when it clears.
	if (!IsCurrentlyStunned())
	{
		if (UBrainComponent* Brain = GetBrainComponent())
		{
			Brain->ResumeLogic(TEXT("Investigating"));
		}
	}
}
