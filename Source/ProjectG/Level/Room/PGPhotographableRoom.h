// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Level/Room/PGMasterRoom.h"
#include "Interface/PhotographableInterface.h"
#include "PGPhotographableRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPhotographableRoom : public APGMasterRoom, public IPhotographableInterface
{
	GENERATED_BODY()

public:
    APGPhotographableRoom();

    // IPhotographableInterface~
    virtual bool ShouldTrackFocusFrame() const override { return false; }
    virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
    virtual FVector GetPhotoTargetLocation() const override;
    // ~IPhotographableInterface

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
    TObjectPtr<UBoxComponent> PhotoTargetBox;	
};
