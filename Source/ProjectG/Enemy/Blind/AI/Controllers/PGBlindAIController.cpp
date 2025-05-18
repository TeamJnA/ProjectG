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

	if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
	{
		CalculateNoise(Stimulus.Strength, Stimulus.StimulusLocation);
	}

	else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Touch>())
	{
		UE_LOG(LogTemp, Log, TEXT(" I FOUND YOU ATTACK!!"));
	}

	/*
	if (Actor && Actor->ActorHasTag(FName("TestPlayer")))
	{
		//CanSeePlayer 키에 true 또는 false 값을 설정합니다. stimulus.WasSuccessfullySensed()는: 대상 방금 감지되었을 때 : true, 사라지면 false
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}*/
}



void APGBlindAIController::CalculateNoise(float Noise, FVector SourceLocation)
{
	
	
	float Distance = FVector::DistSquared(OwnerPawn->GetActorLocation(), SourceLocation); //역제곱법칙

	float CurNoise = Noise / (Distance + 0.1f) * 100000.f;

	float DetectedMaxNoiseMagnitude = GetBlackboardComponent()->GetValueAsFloat("DetectedMaxNoiseMagnitude");

	if (DetectedMaxNoiseMagnitude < CurNoise)
	{
		GetBlackboardComponent()->SetValueAsFloat("DetectedMaxNoiseMagnitude", CurNoise);
		
		
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("cur noise : %.1f"), CurNoise));

		
		GetBlackboardComponent()->SetValueAsVector("TargetLocation", SourceLocation);
		if (OwnerPawn->GetNoiseLevelThreshold() < CurNoise)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Strong Clue")));
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindChase::StaticClass(), true);
		}
		else if(OwnerPawn->GetNoiseLevelThreshold() >= CurNoise)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				FString::Printf(TEXT("Weak Clue")));
			OwnerPawn->GetAbilitySystemComponent()->TryActivateAbilityByClass(UGA_BlindInvestigate::StaticClass(), true);

			
		}

		int flipsign = (GetBlackboardComponent()->GetValueAsInt("BehaviorFlipSign")) * (-1)  ;
		GetBlackboardComponent()->SetValueAsInt("BehaviorFlipSign",flipsign );

		
	}

}


