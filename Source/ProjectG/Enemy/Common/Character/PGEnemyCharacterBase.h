// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Enemy/AI/ETC/PGPatrolPath.h"
#include "Character/PGCharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "PGEnemyCharacterBase.generated.h"

class UPGEnemyAttributeSet;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyCharacterBase : public APGCharacterBase
{
	GENERATED_BODY()
	
	
	
public:
	APGEnemyCharacterBase();

	UBehaviorTree* GetBehaviorTree() const;
	
	void SetMovementSpeed(float speed);





	//삭제 예정
	float GetMovementSpeed() const;

	


	//APGPatrolPath* GetPatrolPath() const;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	UBehaviorTree* Tree;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGEnemyAttributeSet> EnemyAttributeSet;



	//삭제 예정

	/*
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	APGPatrolPath* PatrolPath;*/

};



