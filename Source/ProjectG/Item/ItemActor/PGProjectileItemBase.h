// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/PGItemActor.h"
#include "PGProjectileItemBase.generated.h"

class UProjectileMovementComponent;
class UArrowComponent;

UCLASS()
class PROJECTG_API APGProjectileItemBase : public APGItemActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGProjectileItemBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow")
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TObjectPtr<UShapeComponent> CollisionComponent;

	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;
	*/

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName ItemHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName ItemThrowSound;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float InitialSpeed;

	UFUNCTION(Server, Reliable)
	virtual void PlaySound(const FVector& HitLocation);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> HitSound;
};
