// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Animation/AN_PlayerFootStep.h"

#include "Character/Component/PGSoundManagerComponent.h"
#include "Character/PGCharacterBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#include "PGLogChannels.h"

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

	// 특수한 재질이 바닥에 있어서 추가적인 소리를 내야 하는 지 확인.
	FVector Start = OwnerActor->GetActorLocation();
	FVector UpVector = OwnerActor->GetActorUpVector();
	FVector End = Start + (UpVector * -200.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	Params.bReturnPhysicalMaterial = true;

	if (!OwnerActor->GetWorld())
	{
		return;
	}

	bool bHit = OwnerActor->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_FootStep,
		Params
	);

	UPGSoundManagerComponent* PGSMComp = PGCB->GetSoundManagerComponent();
	if (!PGSMComp)
	{
		return;
	}

	if (bHit && HitResult.PhysMaterial.IsValid())
	{
		if (HitResult.PhysMaterial->SurfaceType == EPhysicalSurface::SurfaceType1)
		{
			if (bIsPlayer)
			{
				PGSMComp->TriggerSoundWithNoise(FName(TEXT("CHARACTER_FootStep_Metal")), OwnerActor->GetActorLocation(), false);
			}
			else
			{
				PGSMComp->TriggerSoundForAllPlayers(FName(TEXT("CHARACTER_FootStep_Metal")), OwnerActor->GetActorLocation());
			}
		}
	}

	// 기본적인 소리 재생
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
