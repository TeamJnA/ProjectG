// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PGMasterRoom.h"
#include "PGStartRoom.generated.h"

class UArrowComponent;

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
	TObjectPtr<UArrowComponent> ExitDir0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDirection")
	TObjectPtr<UArrowComponent> ExitDir3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "OverlapBox")
	TObjectPtr<UBoxComponent> OverlapBox1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RoomMesh")
	TObjectPtr<UChildActorComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDoor")
	TObjectPtr<UChildActorComponent> ExitDoor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick3;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WindowGimmick")
	TObjectPtr<UChildActorComponent> WindowGimmick4;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bonfire")
	TObjectPtr<UChildActorComponent> Bonfire;
};
