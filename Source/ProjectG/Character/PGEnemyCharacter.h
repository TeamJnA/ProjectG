// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/PGPatrolPath.h"
#include "Character/PGCharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameplayTagContainer.h"
#include "PGEnemyCharacter.generated.h"






/**
 * 
 */
UCLASS()
class PROJECTG_API APGEnemyCharacter : public APGCharacterBase
{
	GENERATED_BODY()
	
	
	
public:

	UBehaviorTree* GetBehaviorTree() const;
	
	APGPatrolPath* GetPatrolPath() const;
	
	
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true"))
	UBehaviorTree* Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	APGPatrolPath* PatrolPath;
};




