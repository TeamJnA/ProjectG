// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Ability/Investigate/GA_BlindInvestigate.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "Enemy/Blind/Ability/Chase/GA_BlindChase.h"
#include "Enemy/Blind/Ability/Bite/GA_BlindBite.h"

DEFINE_LOG_CATEGORY(LogEnemy);

APGBlindAIController::APGBlindAIController(FObjectInitializer const& ObjectInitializer) :
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}


void APGBlindAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogEnemy, Log, TEXT("APGBlindAIController::OnPossess"));

	OwnerPawn = Cast<APGBlindCharacter>(InPawn);
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find APGBlindCharacter in APGBlindAIController"));
	}
}


void APGBlindAIController::SetHearingRange(float NewRange)
{
	UE_LOG(LogEnemy, Log, TEXT("APGBlindAIController::SetHearingRange"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = NewRange;
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	}
}

void APGBlindAIController::SetHearingEnabled(bool Enable)
{
	UE_LOG(LogEnemy, Log, TEXT("APGBlindAIController::SetHearingEnabled"));
	if (HearingConfig)
	{
		GetPerceptionComponent()->SetSenseEnabled(UAISense_Hearing::StaticClass(), Enable);
	}
}

void APGBlindAIController::ResetHuntLevel()
{
	UE_LOG(LogEnemy, Log, TEXT("APGBlindAIController::ResetHuntLevel"));
	GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", -1.f);
	OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_Exploration::StaticClass(), true);
	OwnerPawn->SetHuntLevel(0);
}

void APGBlindAIController::SetupPerceptionSystem()
{
	UE_LOG(LogEnemy, Log, TEXT("APGBlindAIController::SetupPerceptionSystem"));

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
	UE_LOG(LogEnemy, Log, TEXT("[APGBlindAIController::OnTargetDetected] AI Detect noise. NOISE LEVEL %f"), Stimulus.Strength);

	//듣기로 감지된 거면
	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGBlindAIController::OnTargetDetected] AI Detect Noise by Hearing."));
		CalculateNoise(Stimulus.Strength, Stimulus.StimulusLocation);
	}
	//touch로 감지된 거라면
	else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Touch>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGBlindAIController::OnTargetDetected] AI Detect Noise by Touching."));
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", Actor->GetActorLocation());
		OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindBite::StaticClass(), true);
	}
}



void APGBlindAIController::CalculateNoise(float Noise, FVector SourceLocation)
{
	
	//거리를 구하려면 제곱하고 루트씌워야하는데, 소리를 구할때 역제곱법칙때문에 다시 제곱해야해서 루트 안 쓰는 DistSquared 사용
	float Distance = FVector::DistSquared(OwnerPawn->GetActorLocation(), SourceLocation); //역제곱법칙 : 소리는 (거리)^2 에 반비례

	//curnoise = 현재 소리. 역제곱법칙으로 구하는 과정인데, Distance가 0을 방지하기위해 0.1정도 더했고, 
	float CurNoise = Noise / (Distance + 0.1f) * 100000.f;

	//MaxThreshold = 이 정도 기준값 이상이면 그냥 최대소리로 인식.
	float NoiseMaxThreshold = OwnerPawn->GetNoiseMaxThreshold(); //캐릭터에서 가져오고

	// DetectedMaxNoiseMagnitude = 현재 감지되어있는, 가장 큰 소리 레벨. 
	// 해당 값은 블랙보드에서 관리함. 그래서 블랙보드에서 가져옴.
	float DetectedMaxNoiseMagnitude = GetBlackboardComponent()->GetValueAsFloat("DetectedMaxNoiseMagnitude");


	// While in HuntLevel 2 (Chase), keep chasing the target that is making noise
	//방금 들린 소리가 최대 소리라면, 
	// huntlevel은 탐색레벨인데, 애니메이션블루프린트때문에 추가한 변수임. 
	// hunt level 0: explore, 1: investigate, 2: chase
	// if 문이 있는 이유가 상대방이 뛰어갈때 쫓아갈 수 있게 갱신하기 위한 if문
	if (CurNoise > NoiseMaxThreshold && OwnerPawn->GetHuntLevel()==2)
	{
		GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", CurNoise);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("MaxThreshold")));

		GetBlackboardComponent()->SetValueAsVector("TargetLocation", SourceLocation);

		
		return;
	}

	//기존에 인식한 최대소리보다 지금 인식한 소리가 크면 
	//-> 얘를 쫓아가게 바꿔줘야대
	if (DetectedMaxNoiseMagnitude < CurNoise)
	{
		// If now hunt level is 0, open all doors around character.
		if (OwnerPawn->GetHuntLevel() == 0)
		{
			OwnerPawn->ForceOpenDoorsAroundCharacter();
		}

		//갱신하기
		GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", CurNoise);
		
		
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("cur noise : %.1f"), CurNoise));

		//targetlocation도 블랙보드에서 관리. 바꿔줌.
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", SourceLocation);

		// 이게 큰 소린지, 작은소린지 구분하는 if문 
		// 구분하는 기준은, blindcharacter의 noiselevelthreshold 변수를 확인.
		if (OwnerPawn->GetNoiseLevelThreshold() < CurNoise)
		{
			//강한 단서
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Strong Clue")));
			//강한단서니깐, chase ability를 실행시킨다
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindChase::StaticClass(), true);
		}
		//이건 약한단서 
		else if(OwnerPawn->GetNoiseLevelThreshold() >= CurNoise)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Weak Clue")));
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindInvestigate::StaticClass(), true);
		}

		// flipsign 변수는, behavior tree의 "Detect State Change" Blackboard Decorator 에서,
		// 값을 observe 하는데, 값이 바뀔때마다 재평가를 한다. (해당 노드로 가도 되는지.)
		// 그런데 평가 기준이 -2보다 큰지인데, 값은 -1과 1을 반복해서 바뀌므로, 무조건 통과가 된다.
		int flipsign = (GetBlackboardComponent()->GetValueAsInt("BehaviorFlipSign")) * (-1)  ;
		GetBlackboardComponent()->SetValueAsInt("BehaviorFlipSign",flipsign );
	}

}


