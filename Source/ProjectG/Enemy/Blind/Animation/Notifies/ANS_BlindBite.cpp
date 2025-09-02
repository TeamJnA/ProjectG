// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Animation/Notifies/ANS_BlindBite.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"

#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY(LogEnemyANS);

void UANS_BlindBite::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	

	/*
	AActor* OwnerActor = MeshComp->GetOwner();
	
	if (APGBlindCharacter* Blind = Cast<APGBlindCharacter>(OwnerActor))
	{
		if (Blind->BiteCollider)
		{
			Blind->BiteCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
	*/
}


void UANS_BlindBite::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	AActor* OwnerActor = MeshComp->GetOwner();

	if (APGBlindCharacter* Blind = Cast<APGBlindCharacter>(OwnerActor))
	{
		UE_LOG(LogEnemyANS, Log, TEXT("BlineBite Notify Ended"));
		Blind->NotifyAttackEnded();
	}

	Super::NotifyEnd(MeshComp, Animation);
}
