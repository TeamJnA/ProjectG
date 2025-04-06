// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGStartRoom.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGStartRoom : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGStartRoom();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	class UArrowComponent* ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	class UArrowComponent* ExitDir1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	class UArrowComponent* ExitDir2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh5;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh6;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh7;
	
};
