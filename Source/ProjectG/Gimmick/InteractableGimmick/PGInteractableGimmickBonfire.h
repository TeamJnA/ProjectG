// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGInteractableGimmickBonfire.generated.h"

class USphereComponent;
class UBoxComponent;
class UPointLightComponent;
class UGameplayEffect;
class UNiagaraComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGInteractableGimmickBonfire : public APGInteractableGimmickBase
{
	GENERATED_BODY()
	
public:
	APGInteractableGimmickBonfire();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

public:
	// IInteractableActorInterface~
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	void StartBonfire();
	void StopBonfire();

protected:
	void OnHealTick();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	TObjectPtr<USphereComponent> SanityHealAreaSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	TObjectPtr<UBoxComponent> InteractCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	TObjectPtr<UNiagaraComponent> FireEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	TObjectPtr<UPointLightComponent> FireLight; 
	
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BonfireMID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonfire")
	TSubclassOf<UGameplayEffect> SanityHealEffectClass;

	FTimerHandle SanityHealTimerHandle;
	FTimerHandle BoneFireDurationTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonfire")
	float BonfireDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonfire")
	float SanityHealInterval = 1.0f;

	float TargetLightIntensity = 0.0f;
	float TargetEmissiveValue = 0.0f;

	float CurrentLightIntensity = 0.0f;
	float CurrentEmissiveValue = 0.0f;

	float FadeSpeed = 3.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsLit, VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	bool bIsLit = false;

	UFUNCTION()
	void OnRep_IsLit();

	void UpdateBonfireLit();
};
