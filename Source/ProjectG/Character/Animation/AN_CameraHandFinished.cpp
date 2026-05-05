// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_CameraHandFinished.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGCameraComponent.h"
#include "PGLogChannels.h"

void UAN_CameraHandFinished::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	UE_LOG(LogPGAnimNotify, Log, TEXT("Play CameraHandFinished Notify"));
	UE_LOG(LogPGAnimNotify, Log, TEXT("Notify Called by Mesh: %s"), *MeshComp->GetName());

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(OwnerActor);
	if (!PGPC)
	{
		return;
	}

	UPGCameraComponent* CameraComp = PGPC->GetCameraComponent();
	if (!CameraComp)
	{
		return;
	}

	CameraComp->CameraHandAnimFinished();
}
