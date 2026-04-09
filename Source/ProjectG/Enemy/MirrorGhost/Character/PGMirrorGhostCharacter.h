// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/PhotographableInterface.h"
#include "PGMirrorGhostCharacter.generated.h"

class APGPlayerCharacter;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGMirrorGhostCharacter : public ACharacter, public IPhotographableInterface
{
	GENERATED_BODY()

public:
	APGMirrorGhostCharacter();

	// IPhotographableInterface~
	virtual bool IsPhotographable() const override;
	virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
	virtual FVector GetPhotoTargetLocation() const override;
	// ~IPhotographableInterface

	virtual void Tick(float DeltaTime) override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	void SetTargetPlayer(APGPlayerCharacter* InTargetPlayer);
	void SetCameraModeVisible(bool bVisible);

	FORCEINLINE APGPlayerCharacter* GetTargetPlayer() const { return TargetPlayer; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateVisibility();

	void UpdateMovement(float DeltaTime);
	bool IsPlayerLooking() const;
	void JumpscareAndDestroy();

	UFUNCTION()
	void OnRep_IsFrozen();

	UFUNCTION()
	void OnRep_TargetPlayer();

	UPROPERTY(ReplicatedUsing = OnRep_TargetPlayer)
	TObjectPtr<APGPlayerCharacter> TargetPlayer;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> MirrorGhostJumpscareTexture;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MirrorGhostMID;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> AttackEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AttackDistance = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float StopAngleThreshold = 0.6f;

	UPROPERTY(ReplicatedUsing = OnRep_IsFrozen)
	bool bIsFrozen = false;

	bool bCameraModeVisible = false;
};
