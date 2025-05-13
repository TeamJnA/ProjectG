// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Touch.h"


APGBlindAIController::APGBlindAIController(FObjectInitializer const& ObjectInitializer) :
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}

void APGBlindAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("Perception Component")));

	HearingConfig = CreateDefaultSubobject <UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 3000.f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);

	}

	TouchConfig = CreateDefaultSubobject <UAISenseConfig_Touch>(TEXT("Touch Config"));
	if (TouchConfig)
	{
		TouchConfig->DetectionByAffiliation.bDetectEnemies = true;
		TouchConfig->DetectionByAffiliation.bDetectFriendlies = true;
		TouchConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->ConfigureSense(*TouchConfig);
	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGBlindAIController::OnTargetDetected);
}

void APGBlindAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//감지한 Actor가  플레이어 클래스라면.. 후에 수정... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//또 ai stimuli source 추가도 cpp에서 하면 좋겟지만... 지금 불가...

	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                         // Key (-1이면 항상 새 메시지)
			2.0f,                       // 표시 시간 (초)
			FColor::Green,             // 글자 색상
			FString::Printf(TEXT("Strength: %.2f"), Stimulus.Strength) // 출력 문자열
		);
		GEngine->AddOnScreenDebugMessage(
			-1,                         // Key (-1이면 항상 새 메시지)
			2.0f,                       // 표시 시간 (초)
			FColor::Green,             // 글자 색상
			FString::Printf(TEXT("tag: %.s"), *Stimulus.Tag.ToString()) // 출력 문자열
		);

	}

	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer 키에 true 또는 false 값을 설정합니다. stimulus.WasSuccessfullySensed()는: 대상 방금 감지되었을 때 : true, 사라지면 false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
