// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGSearchableSlotBase.generated.h"

UENUM(BlueprintType)
enum class ESlotInteractType : uint8
{
	Draw    UMETA(DisplayName = "Draw (DrawerType : SpawnedItem also move)"),
	Open    UMETA(DisplayName = "Open (DoorType : Item Location is fixed. Open both doors)")
};

UCLASS()
class PROJECTG_API APGSearchableSlotBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGSearchableSlotBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE void SetSlotInteractionType(ESlotInteractType _SlotInteractionType)  { SlotInteractionType = _SlotInteractionType; }

	FORCEINLINE TObjectPtr<USceneComponent> GetItemSpawnPoint() const { return ItemSpawnPoint; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	FName SlotName;

	/*
	* Slot을 Draw or Open으로 타입을 나누고,
	* Draw면 아이템도 같이 이동하도록, Open이면 문 메쉬 두 개를 열도록.
	* 
	Draw 할 때, 아이템 액터의 로케이션 변경은 알아서 레플리케이트됨....(Attach 했을 경우)
	그러면 내부에 아이템 액터를 생성할 때, SetReplicateMovement(false)를 False로 하기.

	멀티플레이 고려 사항
	1. 아이템 Spawn 및 아이템을 Slot에 Attach : 서버에서만 해도 클라에 레플리케이션 됨.
	2. Draw의 경우. 일단 이대로 진행해도 될듯.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	ESlotInteractType SlotInteractionType = ESlotInteractType::Draw;

	// 서랍/문 메쉬. 1을 Root로 쓰자.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SlotMesh1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SlotMesh2;

	// 아이템이 스폰될 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ItemSpawnPoint;

};
