// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/Controllers/PGEnemyAIController.h"
#include "ProjectG/Enemy/Base/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

APGEnemyAIController::APGEnemyAIController(FObjectInitializer const& ObjectInitializer)
{
	SetupPerceptionSystem();
}

void APGEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (APGEnemyCharacterBase* const enemy = Cast<APGEnemyCharacterBase>(InPawn))
	{
		if (UBehaviorTree* const tree = enemy->GetBehaviorTree())
		{
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset,b);
			Blackboard = b;
			RunBehaviorTree(tree);
		}
	}
}

void APGEnemyAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("Perception Component")));

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 500.f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius + 25.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.f;
		SightConfig->SetMaxAge(5.f);
		//마지막으로 본 위치 설정. 해당 위치 내라면 감지한 것으로 처리
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		//기본 센스를 시야로 설정.
		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		//타겟을 감지하거나 놓쳤을때 adddynamic (delegate)
		GetPerceptionComponent()->ConfigureSense(*SightConfig);

	}

	HearingConfig = CreateDefaultSubobject < UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 3000.f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);


	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGEnemyAIController::OnTargetDetected);

}

void APGEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//감지한 Actor가  플레이어 클래스라면.. 후에 수정... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//또 ai stimuli source 추가도 cpp에서 하면 좋겟지만... 지금 불가...
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
