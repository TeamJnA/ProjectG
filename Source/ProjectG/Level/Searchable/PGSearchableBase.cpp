// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Searchable/PGSearchableBase.h"
#include "Net/UnrealNetwork.h"
#include "Item/PGItemActor.h"
#include "Level/Searchable/PGSearchableSlotBase.h"
#include "Components/ArrowComponent.h"

// Sets default values
APGSearchableBase::APGSearchableBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

    MainBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainBodyMesh"));
    MainBodyMesh->SetupAttachment(Root);
}


// Called when the game starts or when spawned
void APGSearchableBase::BeginPlay()
{
	Super::BeginPlay();
	
    // 서버 상에서, Arrow Component 위치에 slot들 생성
    if (HasAuthority())
    {
        InitSlots();
    }
}

APGSearchableSlotBase* APGSearchableBase::GetRandomSlot() const
{
    TArray<APGSearchableSlotBase*> ValidSlots;
    for (APGSearchableSlotBase* Slot : SpawnedSlots)
    {
        if (IsValid(Slot))
        {
            ValidSlots.Add(Slot);
        }
    }

    if (ValidSlots.IsEmpty())
    {
        return nullptr;
    }

    // 무작위로 하나 선택
    const int32 RandomIndex = FMath::RandRange(0, ValidSlots.Num() - 1);
    return ValidSlots[RandomIndex];
}

void APGSearchableBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

}

void APGSearchableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
}

void APGSearchableBase::InitSlots()
{
    if (IsValid(SlotClassToSpawn))
    {
        // 에디터에서 생성한 스폰 위치들을 가져온다.
        TArray<UArrowComponent*> ArrowComps;
        GetComponents<UArrowComponent>(ArrowComps);

        // SlotConfig로 슬롯 개수를 확인하고, 미리 Slot 액터를 저장할 공간들을 마련.
        SpawnedSlots.SetNumZeroed(SlotConfigs.Num());

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        for (UArrowComponent* Arrow : ArrowComps)
        {
            if (!Arrow)
            {
                continue;
            }

            // 컴포넌트 이름의 첫 번째로, 몇 번째 서랍인지를 확인한다.
            FString ArrowName = Arrow->GetName();
            int32 ConfigIndex = ArrowName[0] - '0';

            // 서랍의 개수는 1~9개 사이, 인덱스가 그 범위를 넘지 않도록
            check(ConfigIndex >= 0 && ConfigIndex <= 9);
            check(SlotConfigs.IsValidIndex(ConfigIndex));

            // Config에 저장된 상대 위치로 Spawn하기
            const FSearchableSlotConfig& Config = SlotConfigs[ConfigIndex];

            FTransform FinalSpawnTransform = Config.SlotLocalTransform * Arrow->GetComponentTransform();

            APGSearchableSlotBase* SpawnedSlot = GetWorld()->SpawnActor<APGSearchableSlotBase>(
                SlotClassToSpawn,
                FinalSpawnTransform,
                SpawnParams
            );

            if (SpawnedSlot)
            {
                // SpawnedSlot->AttachToComponent(MainBodyMesh, FAttachmentTransformRules::KeepWorldTransform);
               SpawnedSlot->SetSlotInteractionType(Config.InteractionType);

                if (USceneComponent* SpawnPointComp = SpawnedSlot->GetItemSpawnPoint())
                {
                    SpawnPointComp->SetRelativeTransform(Config.ItemSpawnLocalTransform);
                }

                SpawnedSlots[ConfigIndex] = SpawnedSlot;
            }
        }
    }
}


/*
void ASearchableBase::LinkSpawnedItem(int32 SlotIndex, AActor* NewItem)
{
    // 서버 권한 및 유효성 검증 (안전장치)
    if (!HasAuthority() || !SlotConfigs.IsValidIndex(SlotIndex) || !IsValid(NewItem)) return;

    const FSearchableSlotConfig& Slot = SlotConfigs[SlotIndex];

    // 부착 규칙: 월드에 생성된 현재 위치/회전/스케일을 그대로 유지하면서 부모에게 종속시킴
    FAttachmentTransformRules AttachRules(
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        EAttachmentRule::KeepWorld,
        false
    );

    // 타입에 따른 분기 처리
    if (Slot.InteractionType == ESearchableType::Draw)
    {
        // [Draw (서랍형)] 서랍이 움직일 때 같이 움직여야 하므로,
        // 찾아둔 자식 스태틱 메쉬(서랍 메쉬)에 아이템을 직접 붙여버립니다.
        if (Slot.ChildMeshes.IsValidIndex(0) && Slot.ChildMeshes[0])
        {
            NewItem->AttachToComponent(Slot.ChildMeshes[0], AttachRules);
        }
    }
    else if (Slot.InteractionType == ESearchableType::Open)
    {
        // [Open (문 개폐형)] 문만 회전하고 아이템은 가만히 있어야 하므로,
        // 회전하지 않는 슬롯의 고정 스폰 포인트 컴포넌트에 붙여둡니다.
        if (Slot.ItemSpawnPoint)
        {
            NewItem->AttachToComponent(Slot.ItemSpawnPoint, AttachRules);
        }
    }
}
*/