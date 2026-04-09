// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/PhotographableInterface.h"
#include "Type/PGPhotoTypes.h"
#include "PGPhotoSpot.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTG_API APGPhotoSpot : public AActor, public IPhotographableInterface
{
	GENERATED_BODY()

public:
    APGPhotoSpot();

    // IPhotographableInterface~
    virtual float GetPhotoDetectionRange() const override { return 1500.0f; }
    virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
    virtual FVector GetPhotoTargetLocation() const override;
    // ~IPhotographableInterface
    
    void SetPhotoInfo(int32 InPhotoID, int32 InPhotoScore, const FRotator& InBoxRotation = FRotator::ZeroRotator, const FVector& InBoxExtent = FVector(32.0f));

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_BoxConfig();

    UPROPERTY(ReplicatedUsing = OnRep_BoxConfig)
    FVector ReplicatedBoxExtent = FVector(32.0f);

    UPROPERTY(ReplicatedUsing = OnRep_BoxConfig)
    FRotator ReplicatedBoxRotation = FRotator::ZeroRotator;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UBoxComponent> PhotoTargetBox;

    UPROPERTY(Replicated, EditInstanceOnly, Category = "Camera")
    int32 PhotoID = 0;

    UPROPERTY(Replicated, EditInstanceOnly, Category = "Camera")
    int32 PhotoScore = 0;

};
