// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Animation/Notifies/AN_BlindFootPrint.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Components/CapsuleComponent.h"


void UAN_BlindFootPrint::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }

    APGBlindCharacter* Blind = Cast<APGBlindCharacter>(MeshComp->GetOwner());
    if (!Blind)
    {
        return;
    }

    FVector FootLocation = MeshComp->GetSocketLocation(FootBoneName);
    FootLocation.Z = Blind->GetActorLocation().Z - Blind->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Blind->SpawnFootprint(FootLocation);
}

