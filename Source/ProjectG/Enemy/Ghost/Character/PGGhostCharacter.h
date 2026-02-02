// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "PGGhostCharacter.generated.h"

struct FOnAttributeChangeData;
struct FGameplayTag;
class USphereComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGGhostCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface
{
	GENERATED_BODY()

public:
	APGGhostCharacter();

	void SetTargetPlayerState(APlayerState* InPlayerState);

	FORCEINLINE APlayerState* GetTargetPlayerState() const { return TargetPlayerState; }

	// IPGAIExplorationInterface~
	virtual float GetExplorationRadius() const override { return ExplorationRadius; }
	virtual float GetExplorationWaitTime() const override { return ExplorationWaitTime; }
	// ~IPGAIExplorationInterface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ghost|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	void OnChasingTagChanged(const FGameplayTag Tag, int32 NewCount);

	void OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount);

	void UpdateGhostVisibility();

	UFUNCTION()
	void OnRep_IsChasing();

	UFUNCTION()
	void OnRep_IsAttacking();

	virtual void OnTouchColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UFUNCTION()
	void OnLightExtinguishOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnLightExtinguishOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void TryBindLightEffectEvents();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost|Effect")
	TObjectPtr<USphereComponent> LightExtinguishSphere;

	UPROPERTY(EditDefaultsOnly, Category = "Ghost|Ability")
	TSubclassOf<UGameplayEffect> SanityDecreaseEffectClass;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> TargetPlayerState;

	UPROPERTY()
	TWeakObjectPtr<APlayerState> LocalPlayerStateCache;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ghost|AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ghost|Jumpscare")
	float JumpscareCooldown = 5.0f;

	float LastJumpscareTime = -1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsChasing)
	bool bIsCurrentlyChasing = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsAttacking)
	bool bIsCurrentlyAttacking = false;
};
