// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/PGProjectileItemBase.h"
#include "PGProjectileItemGlass.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGProjectileItemGlass : public APGProjectileItemBase
{
	GENERATED_BODY()

public:
	APGProjectileItemGlass();

	virtual void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit) override;

protected:
	virtual void PlaySound_Implementation(const FVector& HitLocation) override;

private:
	int32 bAlreadyHit : 1;
};
