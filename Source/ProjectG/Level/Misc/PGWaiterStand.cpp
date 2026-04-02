// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/PGWaiterStand.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"
#include "Kismet/KismetArrayLibrary.h"

// Sets default values
APGWaiterStand::APGWaiterStand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    // 1. Root Arrow 설정
    RootArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("RootArrow"));
    RootComponent = RootArrow;

    // 2. Mesh 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(RootComponent);

    // 3. 7개의 Spawn Point(Arrow) 생성
    for (int32 i = 0; i < 7; i++)
    {
        FName PointName = *FString::Printf(TEXT("SpawnPoint%d"), i);
        UArrowComponent* NewPoint = CreateDefaultSubobject<UArrowComponent>(PointName);

        NewPoint->SetupAttachment(RootComponent);
        SpawnPoints.Add(NewPoint);
    }
}

void APGWaiterStand::GetRandomSpawnTransforms(int32 Count, TArray<FTransform>& OutTransforms)
{
    OutTransforms.Empty();

    if (SpawnPoints.Num() == 0)
    {
        return;
    }

    // 입력받은 Count가 전체 개수를 넘지 않도록 보정
    int32 ActualCount = FMath::Clamp(Count, 1, 6);

    // 1. 첫 번째 값은 무조건 0번 포인트 추가
    OutTransforms.Add(SpawnPoints[0]->GetComponentTransform());

    // 만약 1개만 필요하면 여기서 종료
    if (ActualCount == 0)
    {
        return;
    }

    // 2. 나머지 인덱스(1~6) 리스트 생성
    TArray<int32> RemainingIndices;
    for (int32 i = 1; i <= 6; i++)
    {
        RemainingIndices.Add(i);
    }

    // 3. 인덱스 리스트를 랜덤하게 섞음
    for (int32 i = RemainingIndices.Num() - 1; i > 0; i--)
    {
        int32 j = FMath::RandRange(0, i);
        RemainingIndices.Swap(i, j);
    }

    // 4. 섞인 리스트에서 ActualCount개만큼 뽑아서 추가
    for (int32 i = 0; i < ActualCount ; i++)
    {
        int32 TargetIndex = RemainingIndices[i];
        OutTransforms.Add(SpawnPoints[TargetIndex]->GetComponentTransform());
    }
}

void APGWaiterStand::SpawnItems(int32 Count)
{
    TArray<FTransform> SpawnTransforms;
    // 1 + Count 개수만큼 가져 온다.
    GetRandomSpawnTransforms(Count, SpawnTransforms);

    if (SpawnTransforms.Num() == 0)
    {
        return;
    }

    // 데이터 에셋 로드 (경로는 프로젝트 설정에 맞게 멤버 변수 등으로 관리 권장)
    UPGItemData* ReviveKitData = ReviveKitItemDataPath.LoadSynchronous();
    UPGItemData* GlassBottleData = GlassBottleItemDataPath.LoadSynchronous();

    if (!ReviveKitData || !GlassBottleData)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 3. 루프를 돌며 아이템 스폰
    for (int32 i = 0; i < SpawnTransforms.Num(); ++i)
    {
        APGItemActor* NewItem = GetWorld()->SpawnActor<APGItemActor>(
            APGItemActor::StaticClass(),
            SpawnTransforms[i],
            SpawnParams
        );

        if (NewItem)
        {
            // 0번 인덱스면 ReviveKit, 아니면 GlassBottle 데이터 할당
            UPGItemData* TargetData = (i == 0) ? ReviveKitData : GlassBottleData;
            NewItem->InitWithData(TargetData);
        }
    }
}

// Called when the game starts or when spawned
void APGWaiterStand::BeginPlay()
{
	Super::BeginPlay();
	
}
