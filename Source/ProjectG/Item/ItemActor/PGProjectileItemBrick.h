// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/PGProjectileItemBase.h"
#include "PGProjectileItemBrick.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGProjectileItemBrick : public APGProjectileItemBase
{
	GENERATED_BODY()
	
public:
	APGProjectileItemBrick();

	//virtual void BeginPlay() override;

	void ThrowInDirection(const FVector& ShootDirection);

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	virtual void OnHit(
		UPrimitiveComponent* HitComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, 
		FVector NormalImpulse, 
		const FHitResult& Hit) override;
};
