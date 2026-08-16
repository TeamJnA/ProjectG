// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "ActiveGameplayEffectHandle.h"
#include "PGInteractableGimmickBonfire.generated.h"

#define LOCTEXT_NAMESPACE "PGInteraction"

class USphereComponent;
class UBoxComponent;
class UPointLightComponent;
class UGameplayEffect;
class UNiagaraComponent;
class APGPlayerCharacter;

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
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	virtual void InteractionFailed() override;
	// ~IInteractableActorInterface

	void StartBonfire();
	void StopBonfire();

protected:
	UFUNCTION()
	void OnHealAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnHealAreaEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void SetBonfireLit();

	UPROPERTY()
	TMap<TWeakObjectPtr<APGPlayerCharacter>, FActiveGameplayEffectHandle> ActiveHealEffectsMap;

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

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	TObjectPtr<UMaterialInterface> MatchIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	FVector2D MatchIconSize = FVector2D(70.0f, 60.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText FireText = LOCTEXT("Bonfire_Fire", "Fire");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName BoneFireStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName BoneFireLoopSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName BoneFireEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName BoneFireFailedSound;

	FTimerHandle SanityHealTimerHandle;
	FTimerHandle BoneFireDurationTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonfire")
	float BonfireDuration = 10.0f;

	float TargetLightIntensity = 0.0f;
	float TargetEmissiveValue = 0.0f;

	float CurrentLightIntensity = 0.0f;
	float CurrentEmissiveValue = 0.0f;

	float FadeSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Bonfire|Vignette")
	float VignetteUpdateInterval = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Bonfire|Vignette")
	float VignetteIntensity = 0.6f;

	UPROPERTY(ReplicatedUsing = OnRep_IsLit, VisibleAnywhere, BlueprintReadOnly, Category = "Bonfire")
	bool bIsLit = false;

	UFUNCTION()
	void OnRep_IsLit();
};

#undef LOCTEXT_NAMESPACE
