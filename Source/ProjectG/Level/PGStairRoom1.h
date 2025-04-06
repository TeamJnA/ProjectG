// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGStairRoom1.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGStairRoom1 : public APGMasterRoom
{
	GENERATED_BODY()

public:
	APGStairRoom1();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	class UArrowComponent* ExitDir0;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh8;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh9;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh10;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh11;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh12;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh13;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh14;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh15;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh16;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh17;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh18;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh19;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh20;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh21;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh22;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh23;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh24;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh25;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh26;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh27;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	class UStaticMeshComponent* Mesh28;
};
