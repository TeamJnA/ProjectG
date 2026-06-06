// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "Interface/HoldInteractProgressHandler.h"
#include "PGInteractableGimmickLever.generated.h"

class APGMirrorRoom;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGInteractableGimmickLever : public APGInteractableGimmickBase, public IHoldInteractProgressHandler
{
	GENERATED_BODY()
	
public:
	APGInteractableGimmickLever();

	void SetMasterRoom(APGMirrorRoom* InRoom);

	// IInteractableActorInterface~
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	// ~IInteractableActorInterface

	void ActivateLever();

	// IHoldInteractProgressHandler~
	virtual void UpdateHoldProgress(float Progress, AActor* Investigator) override;
	virtual void StopHoldProress() override;
	// ~IHoldInteractProgressHandler

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Glass")
	TObjectPtr<UStaticMeshComponent> GlassPlane;

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCrackStage(int32 Stage);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnInteractionComplete();

	void DisableShakeEffect();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName FrameFallSound = FName("LEVEL_MirrorRoom_FrameFall");

	UPROPERTY(EditDefaultsOnly, Category = "Crack")
	FName CrackScale = FName("CrackScale");

	UPROPERTY(EditDefaultsOnly, Category = "Crack")
	FName HoleAmountParamName = FName("HoleAmount");

	UPROPERTY(EditDefaultsOnly, Category = "Shake")
	FName ShakeParameterName = FName("WPOPower");

	UPROPERTY(EditDefaultsOnly, Category = "PaintFade")
	FName PaintFadeParameterName = FName("FadeAmount");

	UPROPERTY()
	TWeakObjectPtr<APGMirrorRoom> MasterRoom;

	UPROPERTY(EditDefaultsOnly, Category = "Crack")
	TArray<float> CrackStageThresholds = { 0.33f, 0.66f, 1.0f };

	UPROPERTY(EditDefaultsOnly, Category = "Crack")
	TArray<float> CrackStageValues = { 0.8f, 1.6f, 2.4f };

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CrackMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PaintMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FrameMID;

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText BreakText = FText::FromString(TEXT("Break"));

	float ShakeIntensity = 0.5f;
	float ShakeDuration = 0.1f;

	int32 CurrentCrackStage = -1;

	UPROPERTY(Replicated)
	bool bIsActivated = false;
};
