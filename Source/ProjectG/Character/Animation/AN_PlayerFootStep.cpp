// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_PlayerFootStep.h"

#include "Character/Component/PGSoundManagerComponent.h"

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

	if (!OwnerActor->HasAuthority())
	{
		return;
	}

	if (UPGSoundManagerComponent* SoundManagerComp = OwnerActor->FindComponentByClass<UPGSoundManagerComponent>())
	{
		// Player sound is need noise, but Enemy sound only spread to players.
		if (bIsPlayer)
		{
			SoundManagerComp->TriggerSoundWithNoise(FootStepName, OwnerActor->GetActorLocation(), true);
		}
		else
		{
			SoundManagerComp->TriggerSoundForAllPlayers(FootStepName, OwnerActor->GetActorLocation());
		}
	}

}
