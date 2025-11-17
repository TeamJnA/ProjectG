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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	void OnChasingTagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnRep_IsChasing();

	virtual void OnTouchColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> TargetPlayerState;

	UPROPERTY(ReplicatedUsing = OnRep_IsChasing)
	bool bIsCurrentlyChasing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ghost Effect")
	TObjectPtr<USphereComponent> LightExtinguishSphere;

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

	UPROPERTY()
	TWeakObjectPtr<APlayerState> LocalPlayerStateCache;
};
