// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Searchable/PGSearchableBase.h"
#include "Net/UnrealNetwork.h"
#include "Item/PGItemActor.h"
#include "Level/Searchable/PGSearchableSlotBase.h"
#include "Components/ArrowComponent.h"
#include "TimerManager.h"

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
    MainBodyMesh->SetCollisionObjectType(ECC_WorldStatic);

    CurrentSlotCount = SlotConfigs.Num();
}


// Called when the game starts or when spawned
void APGSearchableBase::BeginPlay()
{
	Super::BeginPlay();
	
    CurrentSlotCount = SlotConfigs.Num();

    // HighlightOn
    if (MainBodyMesh)
    {
        MainBodyMesh->SetCustomDepthStencilValue(0);
        MainBodyMesh->SetRenderCustomDepth(true);
    }

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

bool APGSearchableBase::GetRandomSlot(APGSearchableSlotBase*& OutSlot, const FRandomStream& InStream)
{
    if (SpawnedSlots.IsEmpty())
    {
        return false;
    }

    // 무작위로 하나 선택
    const int32 RandomIndex = InStream.RandRange(0, SpawnedSlots.Num() - 1);
    if (!IsValid(SpawnedSlots[RandomIndex]))
    {
        return false;
    }

    OutSlot = SpawnedSlots[RandomIndex];
    SpawnedSlots.RemoveAtSwap(RandomIndex);
    CurrentSlotCount--;

    return (CurrentSlotCount != 0);
}

void APGSearchableBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

}

void APGSearchableBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGSearchableBase, SpawnedSlots);
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
                // Slot의 Type 지정, Slot의 Item Spawn Point 지정, Slot을 배열에 저장
                // SpawnedSlot->AttachToComponent(MainBodyMesh, FAttachmentTransformRules::KeepWorldTransform);
               SpawnedSlot->SetSlotInteractionType(Config.InteractionType);

               SpawnedSlot->SetCurrentSlotMesh(Config.SlotMeshType);

               SpawnedSlot->SetItemSpawnPointTransform(Config.ItemSpawnLocalTransform);

                SpawnedSlots[ConfigIndex] = SpawnedSlot;
            }
        }

        OnRep_SpawnedSlots();
    }
}

void APGSearchableBase::HighlightOff()
{
    MainBodyMesh->SetRenderCustomDepth(false);

    for (APGSearchableSlotBase* Slot : SpawnedSlots)
    {
        if (IsValid(Slot) && Slot->OnHighlightOnDelegate.IsBound())
        {
            Slot->OnHighlightOnDelegate.Unbind();
        }
    }
}

void APGSearchableBase::OnRep_SpawnedSlots()
{
    bool bAllSlotsReady = true;

    for (APGSearchableSlotBase* Slot : SpawnedSlots)
    {
        // 슬롯 액터가 클라이언트에 완전히 도착했는지 확인
        if (IsValid(Slot))
        {
            // 이미 바인딩 되어 있어도 덮어쓰므로 중복 바인딩 문제 없음
            Slot->OnHighlightOnDelegate.BindUObject(this, &APGSearchableBase::HighlightOff);

            // UE_LOG(LogTemp, Log, TEXT("APGSearchableBase Delegate Bound! Slot Name: %s, This Name: %s"), *Slot->GetName(), *this->GetName());
        }
        else
        {
            // 배열 정보는 왔는데, 슬롯 액터는 아직 오지 않은 경우
            bAllSlotsReady = false;
        }
    }

    // 도착하지 않은 슬롯이 하나라도 있다면, 0.1초 뒤에 이 함수를 다시 실행. 최대 10번 트라이
    if (!bAllSlotsReady)
    {
        SlotBindRetryCount++;

        if (SlotBindRetryCount >= 10)
        {
            UE_LOG(LogTemp, Warning, TEXT("[APGSearchableBase] Failed to rep slots"));

            SlotBindRetryCount = 0;
            return;
        }

        GetWorldTimerManager().SetTimer(SlotBindRetryTimer, this, &APGSearchableBase::OnRep_SpawnedSlots, 0.1f, false);
    }
}