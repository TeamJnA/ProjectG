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

	// ���İ��� ������ ƨ�� ���, ���� ��� Ƚ���� ������ �д�.
	float LastBounceTime;
	float PlaySoundCoolTime;

	// Ư�� ���� �̻��� �浹�̾�� �Ҹ� ���
	float MinBounceImpact;
};
