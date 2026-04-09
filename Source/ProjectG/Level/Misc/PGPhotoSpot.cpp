// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/PGPhotoSpot.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"


APGPhotoSpot::APGPhotoSpot()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    PhotoTargetBox = CreateDefaultSubobject<UBoxComponent>(TEXT("PhotoTargetBox"));
    PhotoTargetBox->SetupAttachment(RootComponent);
    PhotoTargetBox->SetBoxExtent(FVector(32.0f));
    PhotoTargetBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PhotoTargetBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    PhotoTargetBox->SetCollisionResponseToChannel(ECC_GameTraceChannel7, ECR_Block);
    PhotoTargetBox->SetCollisionResponseToChannel(ECC_GameTraceChannel8, ECR_Overlap);
    PhotoTargetBox->SetGenerateOverlapEvents(true);
    PhotoTargetBox->SetHiddenInGame(true);
}

void APGPhotoSpot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGPhotoSpot, PhotoID);
    DOREPLIFETIME(APGPhotoSpot, PhotoScore);
    DOREPLIFETIME(APGPhotoSpot, ReplicatedBoxExtent);
    DOREPLIFETIME(APGPhotoSpot, ReplicatedBoxRotation);
}

FPhotoSubjectInfo APGPhotoSpot::GetPhotoSubjectInfo() const
{
    return FPhotoSubjectInfo(PhotoID, PhotoScore);
}

FVector APGPhotoSpot::GetPhotoTargetLocation() const
{
    return GetActorLocation();
}

void APGPhotoSpot::SetPhotoInfo(int32 InPhotoID, int32 InPhotoScore, const FRotator& InBoxRotation, const FVector& InBoxExtent)
{
    PhotoID = InPhotoID;
    PhotoScore = InPhotoScore;
    ReplicatedBoxRotation = InBoxRotation;
    ReplicatedBoxExtent = InBoxExtent;
    OnRep_BoxConfig();
}

void APGPhotoSpot::OnRep_BoxConfig()
{
    if (PhotoTargetBox)
    {
        PhotoTargetBox->SetRelativeRotation(ReplicatedBoxRotation);
        PhotoTargetBox->SetBoxExtent(ReplicatedBoxExtent);
    }
}
