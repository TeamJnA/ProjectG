// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/Controllers/PGBlindAIController.h"

#include "ProjectG/Enemy/Base/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"

APGBlindAIController::APGBlindAIController(FObjectInitializer const& ObjectInitializer) :
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}

void APGBlindAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("Perception Component")));

	HearingConfig = CreateDefaultSubobject < UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 3000.f;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);

	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGBlindAIController::OnTargetDetected);
}

void APGBlindAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//������ Actor��  �÷��̾� Ŭ�������.. �Ŀ� ����... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//�� ai stimuli source �߰��� cpp���� �ϸ� ��������... ���� �Ұ�...
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer Ű�� true �Ǵ� false ���� �����մϴ�. stimulus.WasSuccessfullySensed()��: ��� ��� �����Ǿ��� �� : true, ������� false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
