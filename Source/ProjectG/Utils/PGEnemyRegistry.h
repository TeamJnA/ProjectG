// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGEnemyRegistry.generated.h"

/**
 * level俊 积己等 enemy甸阑 包府窍绰 subsystem
 */
UCLASS()
class PROJECTG_API UPGEnemyRegistry : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterEnemy(AActor* Enemy);
	void UnregisterEnemy(AActor* Enemy);
	FORCEINLINE const TArray<TWeakObjectPtr<AActor>>& GetEnemies() const { return Enemies; }

private:
	TArray<TWeakObjectPtr<AActor>> Enemies;
};
