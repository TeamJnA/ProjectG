// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISenseConfig_Touch.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Ability/Investigate/GA_BlindInvestigate.h"
#include "Enemy/Common/AbilitySystem/GA_Exploration.h"
#include "Enemy/Blind/Ability/Chase/GA_BlindChase.h"
#include "Enemy/Blind/Ability/Bite/GA_BlindBite.h"


APGBlindAIController::APGBlindAIController(FObjectInitializer const& ObjectInitializer) :
	APGEnemyAIControllerBase{ ObjectInitializer }
{
	SetupPerceptionSystem();
}


void APGBlindAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	OwnerPawn = Cast<APGBlindCharacter>(InPawn);
}


void APGBlindAIController::SetHearingRange(float NewRange)
{
	if (HearingConfig)
	{
		HearingConfig->HearingRange = NewRange;
		GetPerceptionComponent()->ConfigureSense(*HearingConfig);
	}
}

void APGBlindAIController::SetHearingEnabled(bool Enable)
{
	if (HearingConfig)
	{
		GetPerceptionComponent()->SetSenseEnabled(UAISense_Hearing::StaticClass(), Enable);
	}
}

void APGBlindAIController::ResetHuntLevel()
{
	GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", -1.f);
	OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_Exploration::StaticClass(), true);
	OwnerPawn->SetHuntLevel(0);
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
	//���� ������ �Ÿ�
	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		CalculateNoise(Stimulus.Strength, Stimulus.StimulusLocation);
	}
	//touch�� ������ �Ŷ��
	else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Touch>())
	{
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", Actor->GetActorLocation());
		OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindBite::StaticClass(), true);
	}

	/*
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer Ű�� true �Ǵ� false ���� �����մϴ�. stimulus.WasSuccessfullySensed()��: ��� ��� �����Ǿ��� �� : true, ������� false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}*/
}



void APGBlindAIController::CalculateNoise(float Noise, FVector SourceLocation)
{
	
	//�Ÿ��� ���Ϸ��� �����ϰ� ��Ʈ�������ϴµ�, �Ҹ��� ���Ҷ� ��������Ģ������ �ٽ� �����ؾ��ؼ� ��Ʈ �� ���� DistSquared ���
	float Distance = FVector::DistSquared(OwnerPawn->GetActorLocation(), SourceLocation); //��������Ģ : �Ҹ��� (�Ÿ�)^2 �� �ݺ��

	//curnoise = ���� �Ҹ�. ��������Ģ���� ���ϴ� �����ε�, Distance�� 0�� �����ϱ����� 0.1���� ���߰�, 
	float CurNoise = Noise / (Distance + 0.1f) * 100000.f;

	//MaxThreshold = �� ���� ���ذ� �̻��̸� �׳� �ִ�Ҹ��� �ν�.
	float MaxThreshold = OwnerPawn->GetNoiseMaxThreshold(); //ĳ���Ϳ��� ��������

	// DetectedMaxNoiseMagnitude = ���� �����Ǿ��ִ�, ���� ū �Ҹ� ����. 
	// �ش� ���� �����忡�� ������. �׷��� �����忡�� ������.
	float DetectedMaxNoiseMagnitude = GetBlackboardComponent()->GetValueAsFloat("DetectedMaxNoiseMagnitude");


	//��� �鸰 �Ҹ��� �ִ� �Ҹ����, 
	// huntlevel�� Ž�������ε�, �ִϸ��̼Ǻ������Ʈ������ �߰��� ������. 
	// hunt level 0: explore, 1: investigate, 2: chase
	// if ���� �ִ� ������ ������ �پ�� �Ѿư� �� �ְ� �����ϱ� ���� if��
	if (CurNoise > MaxThreshold && OwnerPawn->GetHuntLevel()==2)
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


