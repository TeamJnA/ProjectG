// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Tasks/PGBTTask_RunGameplayAbility.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"

UPGBTTask_RunGameplayAbility::UPGBTTask_RunGameplayAbility()
{
	NodeName = TEXT("Run Gameplay Ability");

	bIgnoreRestartSelf = true;
}

EBTNodeResult::Type UPGBTTask_RunGameplayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!AbilityToRun)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: No Ability to run."));
		return EBTNodeResult::Failed;
	}

	AAIController* const AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: No valid AIController."));
		return EBTNodeResult::Failed;
	}

	APawn* const Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: No valid pawn."));
		return EBTNodeResult::Failed;
	}

	IAbilitySystemInterface* const ASI = Cast<IAbilitySystemInterface>(Pawn);
	if (!ASI)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: No valid AbilitySystemInterface"));
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* const ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: No valid AbilitySystemComponent"));
		return EBTNodeResult::Failed;
	}

	const bool bSuccess = ASC->TryActivateAbilityByClass(AbilityToRun, true);
	if (bSuccess)
	{
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Warning, TEXT("UPGBTTask_RunGameplayAbility: Failed to activate ability. (%s)"), *AbilityToRun->GetName());
	return EBTNodeResult::Failed;
}

FString UPGBTTask_RunGameplayAbility::GetStaticDescription() const
{
	if (AbilityToRun)
	{
		return FString::Printf(TEXT("%s: %s"), *Super::GetStaticDescription(), *AbilityToRun->GetName());
	}
	return FString::Printf(TEXT("%s: (None)"), *Super::GetStaticDescription());
}
