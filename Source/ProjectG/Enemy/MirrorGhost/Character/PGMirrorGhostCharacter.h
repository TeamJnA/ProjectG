// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PGMirrorGhostCharacter.generated.h"

class APGPlayerCharacter;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGMirrorGhostCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APGMirrorGhostCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	void SetTargetPlayer(APGPlayerCharacter* InTargetPlayer);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateMovement(float DeltaTime);
	bool IsPlayerLookingAtMe() const;
	void JumpscareAndDestroy();

	UPROPERTY()
	TObjectPtr<APGPlayerCharacter> TargetPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> AttackEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float AttackDistance = 80.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float StopAngleThreshold = 0.7f;

	UPROPERTY(ReplicatedUsing = OnRep_IsFrozen)
	bool bIsFrozen = false;

	UFUNCTION()
	void OnRep_IsFrozen();
};
