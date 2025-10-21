// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_SetItemMeshOnHand.h"
#include "Interface/HandItemInterface.h"
#include "PGLogChannels.h"

UAN_SetItemMeshOnHand::UAN_SetItemMeshOnHand()
{
	bIsVisible = false;
}

void UAN_SetItemMeshOnHand::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	UE_LOG(LogPGAnimNotify, Log, TEXT("Play SetItemMeshOnHand Notify"));

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (IHandItemInterface* HandItemInterface = Cast<IHandItemInterface>(OwnerActor))
	{
		if (bIsVisible)
		{
			HandItemInterface->SetItemMesh(true);
			HandItemInterface->SetRightHandIK();
		}
		else
		{
			HandItemInterface->SetItemMesh(false);
		}
	}
}
