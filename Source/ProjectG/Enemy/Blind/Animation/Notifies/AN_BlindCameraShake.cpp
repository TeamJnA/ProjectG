// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Animation/Notifies/AN_BlindCameraShake.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Components/CapsuleComponent.h"


void UAN_BlindCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

    Blind->TriggerFootstepShake();
}

