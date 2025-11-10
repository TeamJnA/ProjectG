// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/AI/Controllers/PGDeafAIController.h"

#include "Enemy/Deaf/Character/PGDeafCharacter.h"

#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Touch.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"

#include "AbilitySystemComponent.h"
#include "Enemy/Deaf/Ability/Chase/GA_DeafChase.h"

APGDeafAIController::APGDeafAIController(FObjectInitializer const& ObjectInitializer) : 
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}

void APGDeafAIController::SetSightRadius(float NewRadius)
{
	if (sightConfig)
	{
		sightConfig->SightRadius = NewRadius;
		GetPerceptionComponent()->ConfigureSense(*sightConfig);
	}
}

void APGDeafAIController::SetSightAngle(float NewAngle)
{
	if (sightConfig)
	{
		sightConfig->PeripheralVisionAngleDegrees = NewAngle;
		GetPerceptionComponent()->ConfigureSense(*sightConfig);
	}
}

void APGDeafAIController::SetSightEnabled(bool Enable)
{
	if (sightConfig)
	{
		GetPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), Enable);
	}
}

void APGDeafAIController::ResetHuntLevel()
{
	UE_LOG(LogEnemy, Log, TEXT("APGDeafAIController::ResetHuntLevel"));
	//GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", -1.f);
	ownerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_Exploration::StaticClass(), true);
	ownerPawn->SetHuntLevel(EDeafHuntLevel::Exploration);
}

void APGDeafAIController::AssignTargetBySight(FVector targetLocation)
{
	GetBlackboardComponent()->SetValueAsVector("TargetLocation", targetLocation);

	if (ownerPawn->GetHuntLevel() == EDeafHuntLevel::Chase)
		return;

	ownerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_DeafChase::StaticClass(), true);
	int8 flipsign = (GetBlackboardComponent()->GetValueAsInt("BehaviorFlipSign")) * (-1);
	GetBlackboardComponent()->SetValueAsInt("BehaviorFlipSign", flipsign);
}

void APGDeafAIController::SetupPerceptionSystem()
{
	SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(
		TEXT("Perception Component")));

	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (sightConfig)
	{
		sightConfig->SightRadius = 500.f;
		sightConfig->LoseSightRadius = sightConfig->SightRadius + 25.f;
		sightConfig->PeripheralVisionAngleDegrees = 90.f;
		sightConfig->SetMaxAge(5.f);
		//마지막으로 본 위치 설정. 해당 위치 내라면 감지한 것으로 처리
		sightConfig->AutoSuccessRangeFromLastSeenLocation = 1.f;
		sightConfig->DetectionByAffiliation.bDetectEnemies = true;
		sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		sightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		//기본 센스를 시야로 설정.
		GetPerceptionComponent()->SetDominantSense(*sightConfig->GetSenseImplementation());
		//타겟을 감지하거나 놓쳤을때 adddynamic (delegate)
		GetPerceptionComponent()->ConfigureSense(*sightConfig);

	}

	touchConfig = CreateDefaultSubobject <UAISenseConfig_Touch>(TEXT("Touch Config"));
	if (touchConfig)
	{
		touchConfig->DetectionByAffiliation.bDetectEnemies = true;
		touchConfig->DetectionByAffiliation.bDetectFriendlies = true;
		touchConfig->DetectionByAffiliation.bDetectNeutrals = true;
		GetPerceptionComponent()->ConfigureSense(*touchConfig);
	}

	GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &APGDeafAIController::OnTargetDetected);
}

void APGDeafAIController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Sight>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGDeafAIController::OnTargetDetected] AI Detect by Sight."));
		AssignTargetBySight(Stimulus.StimulusLocation);
	}
	//touch로 감지된 거라면
	else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Touch>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGDeafAIController::OnTargetDetected] AI Detect by Touching."));
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", Actor->GetActorLocation());
		//OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindBite::StaticClass(), true);

		//TODO 임시코드. 기존에는 UGA_BlindBite 에서 DetectedPlayer 를 true로 했는데, 일단 해당 로직이 없으니 .. 그런데 bite가 되지도 않았는데 사망처리 되는것..
		//bite 하는 곳 따로, 사망처리하는곳 따로는 이상하다. 구조가 잘못된듯
		
		//Blackboard->SetValueAsBool(FName("DetectedPlayer"), true); //임시 코드 취소 필요없을듯 이거  (다시 꺼줘야하는데 번거롭)
	}
}
//대전제 : 모든 몬스터는 가까이가면 무조건 죽는다. 이게 대전제 항상.  
void APGDeafAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogEnemy, Log, TEXT("APGDeafAIController::OnPossess"));

	ownerPawn = Cast<APGDeafCharacter>(InPawn);
	ensureMsgf(ownerPawn, TEXT("Cannot find APGBlindCharacter in APGDeafAIController"));
}
