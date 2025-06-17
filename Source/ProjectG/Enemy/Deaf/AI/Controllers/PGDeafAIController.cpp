// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/AI/Controllers/PGDeafAIController.h"

#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"

APGDeafAIController::APGDeafAIController(FObjectInitializer const& ObjectInitializer) : 
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}

void APGDeafAIController::SetupPerceptionSystem()
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

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGDeafAIController::OnTargetDetected);
}

void APGDeafAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//감지한 Actor가  플레이어 클래스라면.. 후에 수정... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//또 ai stimuli source 추가도 cpp에서 하면 좋겟지만... 지금 불가...
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer 키에 true 또는 false 값을 설정합니다. stimulus.WasSuccessfullySensed()는: 대상 방금 감지되었을 때 : true, 사라지면 false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
