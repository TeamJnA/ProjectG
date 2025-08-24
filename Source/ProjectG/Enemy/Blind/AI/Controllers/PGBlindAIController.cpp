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

	//���� ������ �Ÿ�
	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGBlindAIController::OnTargetDetected] AI Detect Noise by Hearing."));
		CalculateNoise(Stimulus.Strength, Stimulus.StimulusLocation);
	}
	//touch�� ������ �Ŷ��
	else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Touch>())
	{
		UE_LOG(LogEnemy, Log, TEXT("[APGBlindAIController::OnTargetDetected] AI Detect Noise by Touching."));
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", Actor->GetActorLocation());
		OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindBite::StaticClass(), true);
	}
}



void APGBlindAIController::CalculateNoise(float Noise, FVector SourceLocation)
{
	
	//�Ÿ��� ���Ϸ��� �����ϰ� ��Ʈ�������ϴµ�, �Ҹ��� ���Ҷ� ��������Ģ������ �ٽ� �����ؾ��ؼ� ��Ʈ �� ���� DistSquared ���
	float Distance = FVector::DistSquared(OwnerPawn->GetActorLocation(), SourceLocation); //��������Ģ : �Ҹ��� (�Ÿ�)^2 �� �ݺ��

	//curnoise = ���� �Ҹ�. ��������Ģ���� ���ϴ� �����ε�, Distance�� 0�� �����ϱ����� 0.1���� ���߰�, 
	float CurNoise = Noise / (Distance + 0.1f) * 100000.f;

	//MaxThreshold = �� ���� ���ذ� �̻��̸� �׳� �ִ�Ҹ��� �ν�.
	float NoiseMaxThreshold = OwnerPawn->GetNoiseMaxThreshold(); //ĳ���Ϳ��� ��������

	// DetectedMaxNoiseMagnitude = ���� �����Ǿ��ִ�, ���� ū �Ҹ� ����. 
	// �ش� ���� �����忡�� ������. �׷��� �����忡�� ������.
	float DetectedMaxNoiseMagnitude = GetBlackboardComponent()->GetValueAsFloat("DetectedMaxNoiseMagnitude");


	// While in HuntLevel 2 (Chase), keep chasing the target that is making noise
	//��� �鸰 �Ҹ��� �ִ� �Ҹ����, 
	// huntlevel�� Ž�������ε�, �ִϸ��̼Ǻ������Ʈ������ �߰��� ������. 
	// hunt level 0: explore, 1: investigate, 2: chase
	// if ���� �ִ� ������ ������ �پ�� �Ѿư� �� �ְ� �����ϱ� ���� if��
	if (CurNoise > NoiseMaxThreshold && OwnerPawn->GetHuntLevel()==2)
	{
		GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", CurNoise);
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("MaxThreshold")));

		GetBlackboardComponent()->SetValueAsVector("TargetLocation", SourceLocation);

		
		return;
	}

	//������ �ν��� �ִ�Ҹ����� ���� �ν��� �Ҹ��� ũ�� 
	//-> �긦 �Ѿư��� �ٲ���ߴ�
	if (DetectedMaxNoiseMagnitude < CurNoise)
	{
		// If now hunt level is 0, open all doors around character.
		if (OwnerPawn->GetHuntLevel() == 0)
		{
			OwnerPawn->ForceOpenDoorsAroundCharacter();
		}

		//�����ϱ�
		GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", CurNoise);
		
		
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("cur noise : %.1f"), CurNoise));

		//targetlocation�� �����忡�� ����. �ٲ���.
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", SourceLocation);

		// �̰� ū �Ҹ���, �����Ҹ��� �����ϴ� if�� 
		// �����ϴ� ������, blindcharacter�� noiselevelthreshold ������ Ȯ��.
		if (OwnerPawn->GetNoiseLevelThreshold() < CurNoise)
		{
			//���� �ܼ�
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Strong Clue")));
			//���Ѵܼ��ϱ�, chase ability�� �����Ų��
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindChase::StaticClass(), true);
		}
		//�̰� ���Ѵܼ� 
		else if(OwnerPawn->GetNoiseLevelThreshold() >= CurNoise)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Weak Clue")));
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindInvestigate::StaticClass(), true);
		}

		// flipsign ������, behavior tree�� "Detect State Change" Blackboard Decorator ����,
		// ���� observe �ϴµ�, ���� �ٲ𶧸��� ���򰡸� �Ѵ�. (�ش� ���� ���� �Ǵ���.)
		// �׷��� �� ������ -2���� ū���ε�, ���� -1�� 1�� �ݺ��ؼ� �ٲ�Ƿ�, ������ ����� �ȴ�.
		int flipsign = (GetBlackboardComponent()->GetValueAsInt("BehaviorFlipSign")) * (-1)  ;
		GetBlackboardComponent()->SetValueAsInt("BehaviorFlipSign",flipsign );
	}

}


