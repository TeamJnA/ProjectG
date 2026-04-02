// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGPhotographableRoom.h"
#include "Components/BoxComponent.h"

APGPhotographableRoom::APGPhotographableRoom()
{
    PhotoTargetBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhotoTargetBox"));
    PhotoTargetBox->SetupAttachment(RootComponent);
    PhotoTargetBox->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    PhotoTargetBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PhotoTargetBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    PhotoTargetBox->SetCollisionResponseToChannel(ECC_GameTraceChannel7, ECR_Block);
    PhotoTargetBox->SetGenerateOverlapEvents(false);
    PhotoTargetBox->SetHiddenInGame(true);
}

FPhotoSubjectInfo APGPhotographableRoom::GetPhotoSubjectInfo() const
{
    UE_LOG(LogTemp, Warning, TEXT("PhotographableRoom::GetPhotoSubjectInfo: Override missing in %s"), *GetName());
    return FPhotoSubjectInfo(0, 0);
}

FVector APGPhotographableRoom::GetPhotoTargetLocation() const
{
    if (PhotoTargetBox)
    {
        return PhotoTargetBox->GetComponentLocation();
    }
    return GetActorLocation();
}

