// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGPatrolPath.generated.h"

UCLASS()
class PROJECTG_API APGPatrolPath : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGPatrolPath();

	FVector GetPatrolPoint(int const index) const;
	int Num() const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(MakeEditWidget="true", AllowPrivateAccess="true"))
	TArray<FVector> PatrolPoints;
};
