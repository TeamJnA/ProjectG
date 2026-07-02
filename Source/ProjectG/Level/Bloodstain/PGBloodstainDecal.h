// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGBloodstainDecal.generated.h"

class UDecalComponent;

UCLASS()
class PROJECTG_API APGBloodstainDecal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGBloodstainDecal();

	void InitializeDecal(const FVector& NewDecalSize, UMaterialInterface* NewDecalMaterial);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, Category = "Decal")
	TObjectPtr<UDecalComponent> DecalComponent;

	UPROPERTY(ReplicatedUsing = OnRep_TargetDecalSize)
	FVector TargetDecalSize;

	UPROPERTY(ReplicatedUsing = OnRep_TargetDecalMaterial)
	TObjectPtr<UMaterialInterface> TargetDecalMaterial;

	UFUNCTION()
	void OnRep_TargetDecalSize();

	UFUNCTION()
	void OnRep_TargetDecalMaterial();
};
