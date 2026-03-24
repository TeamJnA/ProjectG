// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGWaiterStand.generated.h"

class UArrowComponent;
class UStaticMeshComponent;

class APGItemActor;
class UPGItemData;

UCLASS()
class PROJECTG_API APGWaiterStand : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGWaiterStand();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void GetRandomSpawnTransforms(int32 Count, TArray<FTransform>& OutTransforms);

	void SpawnItems(int32 Count);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 루트가 될 애로우
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent* RootArrow;

	// 메쉬 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// 7개의 스폰 포인트를 담을 배열
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TArray<UArrowComponent*> SpawnPoints;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSoftObjectPtr<UPGItemData> ReviveKitItemDataPath;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSoftObjectPtr<UPGItemData> GlassBottleItemDataPath;
};
