// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Animation/Notify/ANS_ChargerKill.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "PGLogChannels.h"

void UANS_ChargerKill::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UANS_ChargerKill::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp)
	{
		AActor* OwnerActor = MeshComp->GetOwner();
		if (APGChargerCharacter* Charger = Cast<APGChargerCharacter>(OwnerActor))
		{
			UE_LOG(LogPGAnimNotify, Log, TEXT("ANS_ChargerKill::NotifyEnd: Notify Ended"));

			Charger->NotifyAttackEnded();
		}
	}
	else
	{
		UE_LOG(LogPGAnimNotify, Log, TEXT("ANS_ChargerKill::NotifyEnd: Notify Failed"));

	}

	Super::NotifyEnd(MeshComp, Animation);
}
