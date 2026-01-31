// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_PGCharacterPlaySound.h"
#include "Character/Component/PGSoundManagerComponent.h"
#include "Character/PGCharacterBase.h"
#include "PGLogChannels.h"

UAN_PGCharacterPlaySound::UAN_PGCharacterPlaySound()
{
	SoundName = NAME_None;
	SoundType = EPGANSoundType::AllPlayers;
}

void UAN_PGCharacterPlaySound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		UE_LOG(LogPGAnimNotify, Error, TEXT("Cannot Find OwnerActor in UAN_PGCharacterPlaySound."));
		return;
	}

	if (!OwnerActor->HasAuthority())
	{
		UE_LOG(LogPGAnimNotify, Log, TEXT("Play Anim Notify %s"), *SoundName.ToString());
	}

	APGCharacterBase* PGCB = Cast<APGCharacterBase>(OwnerActor);
	if (!PGCB)
	{
		UE_LOG(LogPGAnimNotify, Error, TEXT("UAN_PGCharacterPlaySound can called only PGCharacterBase."));
		return;
	}

	if (!OwnerActor->HasAuthority() && SoundType == EPGANSoundType::PlayersAndAI)
	{
		return;
	}

	if (UPGSoundManagerComponent* PGSMComp = PGCB->GetSoundManagerComponent())
	{
		if (SoundType == EPGANSoundType::PlayersAndAI)
		{
			PGSMComp->TriggerSoundWithNoise(SoundName, OwnerActor->GetActorLocation());
		}
		else if (SoundType == EPGANSoundType::AllPlayers)
		{
			PGSMComp->TriggerSoundLocally(SoundName, OwnerActor->GetActorLocation());
		}
	}
}
