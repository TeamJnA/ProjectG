// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGMasterRoom.generated.h"

UCLASS()
class PROJECTG_API APGMasterRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGMasterRoom();

protected:
	// Called when the game starts or when spawned
	// virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	class USceneComponent* GeometryFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomDirection")
	class UArrowComponent* RoomDir;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	class USceneComponent* OverlapBoxFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OverlapBox")
	class UBoxComponent* OverlapBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	class USceneComponent* ExitsFolder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Folder")
	class USceneComponent* FloorSpawnPoints;

public:
	virtual USceneComponent* GetExitsFolder();
	virtual USceneComponent* GetOverlapBoxFolder();

};
