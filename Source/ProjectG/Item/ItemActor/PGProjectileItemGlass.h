// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemActor/PGProjectileItemBase.h"
#include "PGProjectileItemGlass.generated.h"

class UNiagaraComponent;
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	TObjectPtr<UNiagaraComponent> GlassBreakFX;

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsBroken)
	FRotator HitRotation;

	UFUNCTION()
	void OnRep_IsBroken();

	int32 bAlreadyHit : 1;
};
