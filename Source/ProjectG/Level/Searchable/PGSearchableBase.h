// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Level/Searchable/PGSearchableSlotBase.h"
#include "PGSearchableBase.generated.h"

class APGItemActor;
class APGSearchableSlotBase;


USTRUCT(BlueprintType)
struct FSearchableSlotConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Config")
    ESlotInteractType InteractionType = ESlotInteractType::Draw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Config")
    ESlotMeshType SlotMeshType = ESlotMeshType::Cabinet_Door;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Config")
    FTransform SlotLocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Config")
    FTransform ItemSpawnLocalTransform = FTransform::Identity;
};

UCLASS(PrioritizeCategories = "Slot")
class PROJECTG_API APGSearchableBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGSearchableBase();

    APGSearchableSlotBase* GetRandomSlot() const;

    // Spawn할 위치를 알려주면서, 만약에 이제 스폰할 자리가 없으면 False를 Return.
    // False일 경우, level generator에서는 더 이상 아이템을 스폰할 공간이 없다고 판단 후 이 Searchable을 스폰 배열에서 제거.
    bool GetRandomSlot(APGSearchableSlotBase*& OutSlot);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void InitSlots();

    void HighlightOff();

    UFUNCTION()
    void OnRep_SpawnedSlots();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMeshComponent> MainBodyMesh;

    // Set slot's properties on editor. Used when slot spawned
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
    TArray<FSearchableSlotConfig> SlotConfigs;

    // 에디터에서 스폰할 서랍장(Slot) 클래스 지정
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slot")
    TSubclassOf<APGSearchableSlotBase> SlotClassToSpawn;

    // 생성된 서랍장들을 관리하는 배열
    UPROPERTY(Transient, ReplicatedUsing = OnRep_SpawnedSlots, VisibleAnywhere, BlueprintReadOnly, Category = "Slot")
    TArray<TObjectPtr<APGSearchableSlotBase>> SpawnedSlots;

    FTimerHandle SlotBindRetryTimer;
    int32 SlotBindRetryCount = 0;

    int32 CurrentSlotCount;
};


/*
APGSearchableBase : 아이템이 보관된 가구들. 서랍을 열거나 문을 여는 상호작용을 통해 내부 아이템을 확인할 수 있다.


*/