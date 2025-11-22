// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "PGTriggerGimmickWindowBlood.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGTriggerGimmickWindowBlood : public APGTriggerGimmickBase
{
	GENERATED_BODY()
	
public:
	APGTriggerGimmickWindowBlood();

protected:
	virtual void BeginPlay() override;
	virtual void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayWindowEffect(const FVector& HitLocation);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TObjectPtr<UMaterialInterface> BloodDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName WindowHitSoundName = FName("SFX_WindowHit");

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicWindowMaterial;
};
