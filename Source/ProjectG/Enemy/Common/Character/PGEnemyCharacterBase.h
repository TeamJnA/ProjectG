// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PGCharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameplayTagAssetInterface.h"

#include "PGEnemyCharacterBase.generated.h"

class UPGEnemyAttributeSet;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyCharacterBase : public APGCharacterBase, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
	
	
public:
	APGEnemyCharacterBase();

	UBehaviorTree* GetBehaviorTree() const;
	
	void SetMovementSpeed(float speed);

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	//주변 감지용 collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	UBoxComponent* TouchCollider;


protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	UBehaviorTree* Tree;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGEnemyAttributeSet> EnemyAttributeSet;



	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);



};



