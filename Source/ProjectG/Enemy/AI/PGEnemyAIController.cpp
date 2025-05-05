// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/AI/PGEnemyAIController.h"
#include "Character/PGEnemyCharacter.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

APGEnemyAIController::APGEnemyAIController(FObjectInitializer const& ObjectInitializer)
{
	SetupPerceptionSystem();
}

void APGEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (APGEnemyCharacter* const enemy = Cast<APGEnemyCharacter>(InPawn))
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
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
			TEXT("Perception Component")));
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
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this,&APGEnemyAIController::OnTargetDetected);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);

	}
}

void APGEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	//������ Actor��  �÷��̾� Ŭ�������.. �Ŀ� ����... 
	//if (auto* const ch = Cast<APGPlayerCharacter>(Actor)) {}
	//�� ai stimuli source �߰��� cpp���� �ϸ� ��������... ���� �Ұ�...
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}
}
