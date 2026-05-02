// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_SetCameraMeshOnHand.h"
#include "Character/PGPlayerCharacter.h"
#include "PGLogChannels.h"

UAN_SetCameraMeshOnHand::UAN_SetCameraMeshOnHand()
{
	bIsVisible = false;
}

void UAN_SetCameraMeshOnHand::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	UE_LOG(LogPGAnimNotify, Log, TEXT("Play SetCameraMeshOnHand Notify"));

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(OwnerActor);
	if (!PGPC)
	{
		return;
	}

	PGPC->AttachItemCameraOnHand(bIsVisible);

}
