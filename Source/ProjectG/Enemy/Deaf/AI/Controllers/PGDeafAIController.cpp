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
		//���������� �� ��ġ ����. �ش� ��ġ ����� ������ ������ ó��
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		//�⺻ ������ �þ߷� ����.
		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		//Ÿ���� �����ϰų� �������� adddynamic (delegate)
		GetPerceptionComponent()->ConfigureSense(*SightConfig);

	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGDeafAIController::OnTargetDetected);
}

void APGDeafAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
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
