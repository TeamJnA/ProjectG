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

	virtual void OnHit(
		UPrimitiveComponent* HitComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, 
		FVector NormalImpulse, 
		const FHitResult& Hit) override;

protected:
	virtual void PlaySound_Implementation(const FVector& HitLocation) override;

	void ConvertIntoItem();

	bool bIsItem;

	FTimerHandle ItemConvertTimer;

	TArray<TObjectPtr<UPrimitiveComponent>> HitComponents;

	bool bAlreadyHit;

	// 순식간에 여러번 튕길 경우, 사운드 재생 횟수에 제한을 둔다.
	float LastBounceTime;
	float PlaySoundCoolTime;

	// 특정 수준 이상의 충돌이어야 소리 재생
	float MinBounceImpact;
};
