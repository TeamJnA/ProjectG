// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/PGEnemyRegistry.h"
#include "PGEnemyRegistry.h"

void UPGEnemyRegistry::RegisterEnemy(AActor* Enemy)
{
	if (Enemy)
	{
		Enemies.AddUnique(Enemy);
	}
}

void UPGEnemyRegistry::UnregisterEnemy(AActor* Enemy)
{
	Enemies.RemoveAllSwap([Enemy](const TWeakObjectPtr<AActor>& Weak)
		{
			return Weak.Get() == Enemy;
		});
}
