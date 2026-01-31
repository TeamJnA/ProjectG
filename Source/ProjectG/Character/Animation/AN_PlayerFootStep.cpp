// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_PlayerFootStep.h"

#include "Character/Component/PGSoundManagerComponent.h"
#include "Character/PGCharacterBase.h"

UAN_PlayerFootStep::UAN_PlayerFootStep()
{
	bIsPlayer = false;
	FootStepName = NAME_None;
}

void UAN_PlayerFootStep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	APGCharacterBase* PGCB = Cast<APGCharacterBase>(OwnerActor);
	if (!PGCB)
	{
		return;
	}

	if (!OwnerActor->HasAuthority())
	{
		return;
	}

	if (UPGSoundManagerComponent* PGSMComp = PGCB->GetSoundManagerComponent())
	{
		// Player sound is need noise, but Enemy sound only spread to players.
		if (bIsPlayer)
		{
			PGSMComp->TriggerSoundWithNoise(FootStepName, OwnerActor->GetActorLocation(), true);
		}
		else
		{
			PGSMComp->TriggerSoundForAllPlayers(FootStepName, OwnerActor->GetActorLocation());
		}
	}

}
