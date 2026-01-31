// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Character/PGCharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameplayTagAssetInterface.h"
#include "GenericTeamAgentInterface.h"

#include "PGEnemyCharacterBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEnemyCharacter, Log, All);

class UPGEnemyAttributeSet;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyCharacterBase : public APGCharacterBase, public IGameplayTagAssetInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
		
public:
	APGEnemyCharacterBase();

	UBehaviorTree* GetBehaviorTree() const;
	
	void SetMovementSpeed(float speed);

	// IGameplayTagAssetInterface~
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	// ~IGameplayTagAssetInterface

	//주변 감지용 collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TObjectPtr<UBoxComponent> TouchCollider;

	// Notify target actor that attack is finished
	void NotifyAttackEnded();

	// IGenericTeamAgentInterface~
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	// ~IGenericTeamAgentInterface

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	UBehaviorTree* Tree;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGEnemyAttributeSet> EnemyAttributeSet;

	// CachedAttackedTarget is used to notify the target that the attack has finished.
	// Usually using AnimNotify to pass attack finish information.
	TObjectPtr<AActor> CachedAttackedTarget;

	UFUNCTION()
	virtual void OnTouchColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	virtual void OnPlayerOverlapped(AActor* OverlapPlayer);

	virtual void OnDoorOverlapped(AActor* OverlapDoor);

	FORCEINLINE void SetDoorBreak(bool NewDoorOpen) { bDoorBreakOpen = NewDoorOpen; }

	bool bDoorBreakOpen;

public:
	void ForceOpenDoorsAroundCharacter();

	// Get Enemy's head location.
	// Enemy's have different head sockets, use the top of the capsule component instead.
	FVector GetCapsuleTopWorldLocation() const;
};



