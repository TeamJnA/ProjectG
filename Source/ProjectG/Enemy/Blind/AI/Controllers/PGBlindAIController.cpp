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
	//������ Actor��  �÷��̾� Ŭ�������.. �Ŀ� ����... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//�� ai stimuli source �߰��� cpp���� �ϸ� ��������... ���� �Ұ�...

	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                         // Key (-1�̸� �׻� �� �޽���)
			2.0f,                       // ǥ�� �ð� (��)
			FColor::Green,             // ���� ����
			FString::Printf(TEXT("Strength: %.2f"), Stimulus.Strength) // ��� ���ڿ�
		);
		GEngine->AddOnScreenDebugMessage(
			-1,                         // Key (-1�̸� �׻� �� �޽���)
			2.0f,                       // ǥ�� �ð� (��)
			FColor::Green,             // ���� ����
			FString::Printf(TEXT("tag: %.s"), *Stimulus.Tag.ToString()) // ��� ���ڿ�
		);

	}

	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer Ű�� true �Ǵ� false ���� �����մϴ�. stimulus.WasSuccessfullySensed()��: ��� ��� �����Ǿ��� �� : true, ������� false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
