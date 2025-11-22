// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ghost/Animation/Notifies/ANS_GhostAttack.h"
#include "Enemy/Ghost/Character/PGGhostCharacter.h"
#include "PGLogChannels.h"

void UANS_GhostAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UANS_GhostAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (APGGhostCharacter* Ghost = Cast<APGGhostCharacter>(OwnerActor))
		{
			UE_LOG(LogPGAnimNotify, Log, TEXT("ANS_GhostAttack::NotifyEnd: Notify Ended"));

			Ghost->NotifyAttackEnded();
		}
	}
	else
	{
		UE_LOG(LogPGAnimNotify, Log, TEXT("ANS_GhostAttack::NotifyEnd: Notify Failed"));

	}

	Super::NotifyEnd(MeshComp, Animation);
}
