// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/AI/Tasks/PGBTTask_PlayExplorationSound.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "AIController.h"


struct FGhostPlaySoundMemory
{
	bool bPlayedOnce = false;
};

uint16 UPGBTTask_PlayExplorationSound::GetInstanceMemorySize() const
{
	return sizeof(FGhostPlaySoundMemory);
}

UPGBTTask_PlayExplorationSound::UPGBTTask_PlayExplorationSound()
{
	NodeName = TEXT("Play Exploration Sound");
}

EBTNodeResult::Type UPGBTTask_PlayExplorationSound::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FGhostPlaySoundMemory* Memory = reinterpret_cast<FGhostPlaySoundMemory*>(NodeMemory);

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return EBTNodeResult::Failed;
	}

	APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(AIC->GetPawn());
	if (!Ghost)
	{
		return EBTNodeResult::Failed;
	}

	// 첫 실행 스킵
	if (bSkipFirst && !Memory->bPlayedOnce)
	{
		Memory->bPlayedOnce = true;
		return EBTNodeResult::Succeeded;
	}

	Memory->bPlayedOnce = true;
	// [Sound] TODO: Exploration 시작 사운드 (AmbientSound 느낌)
	Ghost->PlaySoundToTargetPlayer(SoundName);

	return EBTNodeResult::Succeeded;
}
