// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGSwingProp.generated.h"

class UPGSoundManagerComponent;

UCLASS()
class PROJECTG_API APGSwingProp : public AActor
{
	GENERATED_BODY()
	
public:	
	APGSwingProp();
	virtual void Tick(float DeltaTime) override;
	void SetMaterialVariation(int32 InIndex);
	FORCEINLINE int32 GetMaterialVariationCount() const { return MaterialVariations.Num(); }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StartSwing(FVector_NetQuantizeNormal InSwingAxis, float InAmplitude);

	void StartSwingLocally(const FVector& InSwingAxis, float InAmplitude);

	FVector SwingAxis = FVector::ZeroVector;
	FRotator InitialRotation;

	UPROPERTY(EditDefaultsOnly, Category = "Material")
	TArray<TObjectPtr<UMaterialInterface>> MaterialVariations;

	UPROPERTY(EditAnywhere, Category = "Sound")
	FName HitSoundName = FName("GIMMICK_SwingProp_Hit");

	UPROPERTY(EditAnywhere, Category = "Sound")
	FName SqueakSoundName = FName("GIMMICK_SwingProp_Squeak");

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, Category = "Swing")
	float MaxSwingAngle = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Swing")
	float SwingFrequency = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Swing")
	float SwingDamping = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Swing")
	float HitCooldown = 1.0f;

	float SwingElapsed = 0.0f;
	float CurrentAmplitude = 0.0f;
	float LastHitTime = -1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_MaterialIndex)
	int32 MaterialIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_MaterialIndex();

	bool bIsSwinging = false;
};
