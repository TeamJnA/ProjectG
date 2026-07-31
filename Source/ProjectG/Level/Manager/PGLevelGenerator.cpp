// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Manager/PGLevelGenerator.h"

#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"

#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"

#include "Level/Room/PGMasterRoom.h"
#include "Level/Room/PGStartRoom.h"

#include "Level/Misc/PGDoor1.h"
#include "Level/Misc/PGWall.h"
#include "Level/Misc/PGWaiterStand.h"
#include "Level/Misc/PGHideProp.h"
#include "Level/Searchable/PGSearchableBase.h"
#include "Level/Bloodstain/PGBloodstainSpawnPoint.h"
#include "Level/Bloodstain/PGBloodstainDecal.h"

#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickMannequin.h"
#include "Gimmick/InteractableGimmick/PGFuseBox.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickArmorStand.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Type/PGDifficultyTypes.h"


// Sets default values
APGLevelGenerator::APGLevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// max room spawn amount
	RoomAmount = 23;

	// reload level if (elpased time > max generation time)
	MaxGenerateTime = 10.0f;

	static ConstructorHelpers::FClassFinder<AActor> BlindCharacterRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/BP_BlindCharacter.BP_BlindCharacter_C"));
	if (BlindCharacterRef.Class)
	{
		BlindCharacter = BlindCharacterRef.Class;
	}	
	
	static ConstructorHelpers::FClassFinder<AActor> ChargerCharacterRef(TEXT("/Game/ProjectG/Enemy/Charger/Character/BP_PGChargerCharacter.BP_PGChargerCharacter_C"));
	if (ChargerCharacterRef.Class)
	{
		ChargerCharacter = ChargerCharacterRef.Class;
	}

	static ConstructorHelpers::FClassFinder<AActor> MannequinRef(TEXT("/Game/ProjectG/Gimmick/Trigger/Mannequin/BP_PGTriggerGimmickMannequin.BP_PGTriggerGimmickMannequin_C"));
	if (MannequinRef.Succeeded())
	{
		MannequinClass = MannequinRef.Class;
	}

	// Set base Searchable Class Map
	const UEnum* SearchableEnum = StaticEnum<ESearchableType>();
	if (SearchableEnum)
	{
		for (int32 i = 0; i < SearchableEnum->NumEnums() - 1; ++i)
		{
			ESearchableType EnumValue = static_cast<ESearchableType>(SearchableEnum->GetValueByIndex(i));

			if (!SearchableClassMap.Contains(EnumValue))
			{
				SearchableClassMap.Add(EnumValue, nullptr);
			}
		}
	}
}

/*
* 레벨 생성 시작
* 레벨 내 생성된 Room들의 ExitPoint(ExitPointsList)에서 새로운 Room 생성 시도
* Overlap 검사 실패 시 새로운 ExitPoint를 골라 재생성
* Overlap 검사 성공 시 ExitPointsList에 새로 생긴 Room의 ExitPoint를 추가하여 새로운 ExitPoint 선택/Room 생성
* 모든 Room 생성 후 Room이 연결되지 않은 모든 ExitPoint에 벽 스폰
* Room이 연결된 ExitPoint들 중 선택하여 문 스폰
* Room의 지정된 ItemSpawnPoint들 중 선택하여 아이템 스폰
* 레벨 생성 과정에서 구축한 RoomGraph를 활용해 StartRoom에서 중간 거리의 Room들을 찾아 선택하여 적대 AI 스폰
*/
void APGLevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetSeed();
		SpawnStartRoom();
		SpawnLoopCorridor();

		GenerationStartTime = GetWorld()->GetTimeSeconds();
		StartLevelGenerateTimer();

		SpawnNextRoom();
	}
}

/*
* 시드 설정
* 시드값이 없는 경우 랜덤 초기화
* 시드값을 정한 경우 해당 시드값 사용
*/
void APGLevelGenerator::SetSeed()
{
	if (SeedValue == -1)
	{
		Seed.Initialize(FDateTime::Now().GetTicks());
	}
	else
	{
		Seed.Initialize(SeedValue);
	}
}

/*
* StartRoom 스폰
* RoomGraph 첫 노드로 추가
* 기본 아이템, 탈출구 스폰
*/
void APGLevelGenerator::SpawnStartRoom()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform SpawnTransform = Root->GetComponentTransform();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName StartRoomName = TEXT("StartRoom");
	TSubclassOf<APGMasterRoom> StartRoomClass = RoomClassMap[StartRoomName];
	APGStartRoom* NewRoom = World->SpawnActor<APGStartRoom>(StartRoomClass, SpawnTransform, SpawnParams);
	if (!NewRoom)
	{
		return;
	}
	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
	if (!GI)
	{
		return;
	}

	// Add to RoomGraph
	StartRoom = NewRoom;
	RoomGraph.Add(NewRoom);
	bRoomDepthsDirty = true;

	// MasterRoom .h
	//  const USceneComponent* GetExitPointsFolder() const { return ExitPointsFolder; }
	if (const USceneComponent* ExitPointsFolder = NewRoom->GetExitPointsFolder())
	{
		const TArray<USceneComponent*>& ExitPoints = ExitPointsFolder->GetAttachChildren();
		ExitPointsList.Reserve(ExitPointsList.Num() + ExitPoints.Num());
		ExitPointsList.Append(ExitPoints);
	}

	// Deco spawn
	if (const USceneComponent* BloodstainSpawnPointFolder = NewRoom->GetBloodstainSpawnPointsFolder())
	{
		const TArray<USceneComponent*>& BloodSpawnPoints = BloodstainSpawnPointFolder->GetAttachChildren();
		BloodstainSpawnPointsList.Reserve(BloodstainSpawnPointsList.Num() + BloodSpawnPoints.Num());
		for (USceneComponent* SpawnPoint : BloodSpawnPoints)
		{
			if (UPGBloodstainSpawnPoint* Point = Cast<UPGBloodstainSpawnPoint>(SpawnPoint))
			{
				BloodstainSpawnPointsList.Add(Point);
			}
		}
	}

	// TODO : for test ~ need to remove
	if (const USceneComponent* ArmorStandSpawnPointFolder = NewRoom->GetArmorStandSpawnPointsFolder())
	{
		const TArray<USceneComponent*>& ArmorStandSpawnPoints = ArmorStandSpawnPointFolder->GetAttachChildren();
		ArmorStandSpawnPointsList.Reserve(ArmorStandSpawnPointsList.Num() + ArmorStandSpawnPoints.Num());
		ArmorStandSpawnPointsList.Append(ArmorStandSpawnPoints);
	}

	/*
	// for test ~
	APGItemActor* TestReviveKit0 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ReviveKitItemData0 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit0->InitWithData(ReviveKitItemData0);
	}
	TestReviveKit0->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestReviveKit1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ReviveKitItemData1 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit1->InitWithData(ReviveKitItemData1);
	}
	TestReviveKit1->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestReviveKit2 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ReviveKitItemData2 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit2->InitWithData(ReviveKitItemData2);
	}
	TestReviveKit2->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestReviveKit3 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ReviveKitItemData3 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit3->InitWithData(ReviveKitItemData3);
	}
	TestReviveKit3->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestReviveKit4 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* ReviveKitItemData4 = GI->GetItemDataByKey("ReviveKit"))
	{
		TestReviveKit4->InitWithData(ReviveKitItemData4);
	}
	TestReviveKit4->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestMatch0 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* MatchItemData0 = GI->GetItemDataByKey("Match"))
	{
		TestMatch0->InitWithData(MatchItemData0);
	}
	TestMatch0->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestMatch1 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* MatchItemData1 = GI->GetItemDataByKey("Match"))
	{
		TestMatch1->InitWithData(MatchItemData1);
	}
	TestMatch1->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestMatch2 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* MatchItemData2 = GI->GetItemDataByKey("Match"))
	{
		TestMatch2->InitWithData(MatchItemData2);
	}
	TestMatch2->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestMatch3 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* MatchItemData3 = GI->GetItemDataByKey("Match"))
	{
		TestMatch3->InitWithData(MatchItemData3);
	}
	TestMatch3->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));

	APGItemActor* TestMatch4 = World->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnParams);
	if (UPGItemData* MatchItemData4 = GI->GetItemDataByKey("Match"))
	{
		TestMatch4->InitWithData(MatchItemData4);
	}
	TestMatch4->SetActorRelativeLocation(FVector(2765.0f, 490.0f, 1700.0f));
	*/
}

void APGLevelGenerator::SpawnLoopCorridor()
{
	UWorld* World = GetWorld();
	if (!World || !StartRoom || LoopCorridorClassArray.IsEmpty())
	{
		return;
	}

	EPGDifficulty Diff = EPGDifficulty::Normal;
	if (APGGameState* GS = World->GetGameState<APGGameState>())
	{
		Diff = GS->GetDifficultyLevel();
	}

	// 1층/2층 루프 클래스 분류
	TArray<TSubclassOf<APGMasterRoom>> Floor1Loops;
	TSubclassOf<APGMasterRoom> Floor2Loop = nullptr;
	for (const TSubclassOf<APGMasterRoom>& LoopClass : LoopCorridorClassArray)
	{
		if (!LoopClass)
		{
			continue;
		}

		const APGMasterRoom* DefaultLoop = LoopClass->GetDefaultObject<APGMasterRoom>();
		if (!DefaultLoop)
		{
			continue;
		}

		if (DefaultLoop->IsSecondFloorLoop())
		{
			Floor2Loop = LoopClass;
		}
		else
		{
			Floor1Loops.Add(LoopClass);
		}
	}

	if (Diff == EPGDifficulty::Hard)
	{
		// 2층 확정 + 1층 3개 중 랜덤 1개
		if (Floor2Loop)
		{
			SpawnSingleLoopCorridor(Floor2Loop);
		}
		if (Floor1Loops.Num() > 0)
		{
			const int32 Idx = UKismetMathLibrary::RandomIntegerFromStream(Seed, Floor1Loops.Num());
			SpawnSingleLoopCorridor(Floor1Loops[Idx]);
		}
	}
	else // Normal: 랜덤 1개
	{
		const int32 Total = LoopCorridorClassArray.Num();
		if (Total > 0)
		{
			const int32 Idx = UKismetMathLibrary::RandomIntegerFromStream(Seed, Total);
			SpawnSingleLoopCorridor(LoopCorridorClassArray[Idx]);
		}
	}
}

void APGLevelGenerator::SpawnSingleLoopCorridor(TSubclassOf<APGMasterRoom> LoopClass)
{
	UWorld* World = GetWorld();
	if (!World || !StartRoom || !LoopClass)
	{
		return;
	}

	APGMasterRoom* DefaultLoop = LoopClass->GetDefaultObject<APGMasterRoom>();
	if (!DefaultLoop)
	{
		return;
	}

	const EStartRoomExit StartExitEnum = DefaultLoop->GetLoopStartExit();
	const EStartRoomExit EndExitEnum = DefaultLoop->GetLoopEndExit();
	if (StartExitEnum == EStartRoomExit::None || EndExitEnum == EStartRoomExit::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnLoopCorridor: Loop class has no exit points configured"));
		return;
	}

	APGStartRoom* StartRoomTyped = Cast<APGStartRoom>(StartRoom);
	if (!StartRoomTyped)
	{
		return;
	}

	USceneComponent* StartExit = StartRoomTyped->GetExitPoint(StartExitEnum);
	USceneComponent* EndExit = StartRoomTyped->GetExitPoint(EndExitEnum);
	if (!StartExit || !EndExit)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnLoopCorridor: Failed to find exit points"));
		return;
	}

	const FTransform SpawnTransform(StartExit->GetComponentRotation(), StartExit->GetComponentLocation());
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APGMasterRoom* LoopRoom = World->SpawnActor<APGMasterRoom>(LoopClass, SpawnTransform, SpawnParams);
	if (!LoopRoom)
	{
		return;
	}

	RoomGraph.FindOrAdd(StartRoom).Add(LoopRoom);
	RoomGraph.FindOrAdd(LoopRoom).Add(StartRoom);
	bRoomDepthsDirty = true;

	ExitPointsList.Remove(StartExit);
	ExitPointsList.Remove(EndExit);
	DoorPointsList.Add(StartExit);
	DoorPointsList.Add(EndExit);

	if (const USceneComponent* LoopExitFolder = LoopRoom->GetExitPointsFolder())
	{
		const TArray<USceneComponent*>& LoopExits = LoopExitFolder->GetAttachChildren();
		ExitPointsList.Reserve(ExitPointsList.Num() + LoopExits.Num());
		ExitPointsList.Append(LoopExits);
	}

	// Deco spawn
	if (const USceneComponent* BloodstainSpawnPointFolder = LoopRoom->GetBloodstainSpawnPointsFolder())
	{
		const TArray<USceneComponent*>& BloodSpawnPoints = BloodstainSpawnPointFolder->GetAttachChildren();
		BloodstainSpawnPointsList.Reserve(BloodstainSpawnPointsList.Num() + BloodSpawnPoints.Num());
		for (USceneComponent* SpawnPoint : BloodSpawnPoints)
		{
			if (UPGBloodstainSpawnPoint* Point = Cast<UPGBloodstainSpawnPoint>(SpawnPoint))
			{
				BloodstainSpawnPointsList.Add(Point);
			}
		}
	}
}

/*
* 다음 Room 생성
* RoomAmount > 14(초기단계) -> 복도형 Room만 생성
* RoomAmount <= 14 -> 모든 Room 클래스 중 선택하여 생성
* Room 생성 후 Overlap 검사
*/
void APGLevelGenerator::SpawnNextRoom()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnNextRoom: Generation stopped"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World || ExitPointsList.IsEmpty())
	{
		return;
	}

	const int32 SelectedIndex = SelectExitPointWithBalancing();
	if (SelectedIndex == INDEX_NONE)
	{
		return;
	}
	const TObjectPtr<USceneComponent> SelectedExitPoint = ExitPointsList[SelectedIndex];
	//const TObjectPtr<USceneComponent> SelectedExitPoint = ExitPointsList[UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitPointsList.Num())];
	const FTransform SpawnTransform(SelectedExitPoint->GetComponentRotation(), SelectedExitPoint->GetComponentLocation());

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName TargetRoomName = TEXT("");

	APGMasterRoom* NewRoom = nullptr;
	// Corridor 2
	if (RoomAmount > 21)
{
		TargetRoomName = TEXT("Room1");
	}
	// Corridor_Dark 2
	else if (RoomAmount > 19)
	{
		TargetRoomName = TEXT("Corridor_Dark");
	}
	// StairRoom_Simple 1
	else if (RoomAmount > 18)
	{
		TargetRoomName = TEXT("StairRoom_Simple");
	}
	// Corridor_Simple 1
	else if (RoomAmount > 17)
	{
		TargetRoomName = TEXT("Corridor_Simple");
	}
	// SmallCorridor_Dark 1
	else if (RoomAmount > 16)
	{
		TargetRoomName = TEXT("SmallCorridor_Dark");
	}
	// Library 1
	else if (RoomAmount > 15)
	{
		TargetRoomName = TEXT("LibraryRoom");
	}
	// SmallCorridor 1
	else if (RoomAmount > 14)
	{
		TargetRoomName = TEXT("Room2");
	}
	// DiningRoom 1
	else if (RoomAmount > 13)
	{
		TargetRoomName = TEXT("DiningRoom");
	}
	// Corridor 1
	else if (RoomAmount > 12)
	{
		TargetRoomName = TEXT("Room1");
	}
	// SmallCorridor_Mannequin 1
	else if (RoomAmount > 11)
	{
		TargetRoomName = TEXT("SmallCorridor_Mannequin");
	}
	// Corridor_Dark 1
	else if (RoomAmount > 10)
	{
		TargetRoomName = TEXT("Corridor_Dark");
	}
	// BedRoom 1
	else if (RoomAmount > 9)
	{
		TargetRoomName = TEXT("Room3");
	}
	// Storage 1
	else if (RoomAmount > 8)
	{
		TargetRoomName = TEXT("Storage");
	}
	// Corridor_Simple 1
	else if (RoomAmount > 7)
	{
		TargetRoomName = TEXT("Corridor_Simple");
	}
	// StairRoom 1
	else if (RoomAmount > 6)
	{
		TargetRoomName = TEXT("StairRoom1");
	}
	// SmallCorridor_Bonfire 1
	else if (RoomAmount > 5)
	{
		TargetRoomName = TEXT("SmallCorridor_Bonfire");
	}
	// BarrelRoom 1
	else if (RoomAmount > 4)
	{
		TargetRoomName = TEXT("BarrelRoom");
	}
	// ChargerRoom 1
	else if (RoomAmount > 3)
	{
		TargetRoomName = TEXT("ChargerRoom");
	}
	// GhostRoom 1
	else if (RoomAmount > 2)
	{
		TargetRoomName = TEXT("GhostRoom");
	}
	// BlindRoom 1
	else if (RoomAmount > 1)
	{
		TargetRoomName = TEXT("BlindRoom");
	}
	// ElevatorRoom 1
	else
	{
		TargetRoomName = TEXT("ElevatorRoom");
	}
	TSubclassOf<APGMasterRoom> NewRoomClass = RoomClassMap[TargetRoomName];
	NewRoom = World->SpawnActor<APGMasterRoom>(NewRoomClass, SpawnTransform, spawnParams);

	check(NewRoom);

	TWeakObjectPtr<APGLevelGenerator> WeakThis(this);
	TWeakObjectPtr<APGMasterRoom> WeakNewRoom(NewRoom);
	TWeakObjectPtr<USceneComponent> WeakSelectedExitPoint(SelectedExitPoint);

	//FTimerHandle DelayTimerHandle;
	//World->GetTimerManager().SetTimer(
	//	DelayTimerHandle,
	//	FTimerDelegate::CreateLambda([WeakThis, WeakSelectedExitPoint, WeakNewRoom]()
	//	{
	//		if (WeakThis.IsValid() && WeakSelectedExitPoint.IsValid() && WeakNewRoom.IsValid())
	//		{
	//			WeakThis->CheckOverlap(WeakSelectedExitPoint.Get(), WeakNewRoom.Get());
	//		}
	//	}), 
	//	0.1f,
	//	false
	//);
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateLambda([WeakThis, WeakSelectedExitPoint, WeakNewRoom]()
		{
			if (WeakThis.IsValid() && WeakSelectedExitPoint.IsValid() && WeakNewRoom.IsValid())
			{
				WeakThis->CheckOverlap(WeakSelectedExitPoint.Get(), WeakNewRoom.Get());
			}
		}));
}

int32 APGLevelGenerator::SelectExitPointWithBalancing()
{
	if (ExitPointsList.IsEmpty())
	{
		return INDEX_NONE;
	}

	EnsureRoomDepthMap();

	TArray<int32> Depths;
	Depths.Reserve(ExitPointsList.Num());
	int32 MaxDepth = 0;

	for (const TObjectPtr<USceneComponent>& ExitPoint : ExitPointsList)
	{
		const APGMasterRoom* OwnerRoom = ExitPoint ? Cast<APGMasterRoom>(ExitPoint->GetOwner()) : nullptr;
		const int32 Depth = GetRoomDepthFromStart(OwnerRoom);
		Depths.Add(Depth);
		MaxDepth = FMath::Max(MaxDepth, Depth);
	}

	TArray<float> Weights;
	Weights.Reserve(ExitPointsList.Num());
	float TotalWeight = 0.0f;

	for (int32 Depth : Depths)
	{
		const float W = FMath::Pow((float)(MaxDepth - Depth + 1), 2.0f);
		Weights.Add(W);
		TotalWeight += W;
	}

	if (TotalWeight <= 0.0f)
	{
		return UKismetMathLibrary::RandomIntegerFromStream(Seed, ExitPointsList.Num());
	}

	const float Roll = UKismetMathLibrary::RandomFloatInRangeFromStream(Seed, 0.0f, TotalWeight);
	float Accumulated = 0.0f;
	for (int32 i = 0; i < ExitPointsList.Num(); ++i)
	{
		Accumulated += Weights[i];
		if (Roll <= Accumulated)
		{
			return i;
		}
	}

	return ExitPointsList.Num() - 1;
}

int32 APGLevelGenerator::GetRoomDepthFromStart(const APGMasterRoom* Room) const
{
	if (!Room)
	{
		return 0;
	}

	const int32* DepthPtr = RoomDepths.Find(const_cast<APGMasterRoom*>(Room));
	return DepthPtr ? *DepthPtr : 0;
}

/*
* 새로운 Room에 대한 Overlap 검사 및 이후 동작
* Overlap 새로 생성된 Room을 지우고 다시 생성
* Overlap x -> RoomGraph에 추가 -> 부모 Room의 인접 Room으로 추가, 본인의 인접 Room으로 부모 Room 추가
* 사용된 ExitPoint를 ExitPointsList에서 제거, DoorPointsList에 추가
* 생성 확정된 Room의 ExitPoints를 ExitPointsList에 추가,
* 생성 확정된 Room의 Searchable를 SearchableSpawnPointsList에 추가
* RoomAmount가 남은 경우 다음 Room 생성
* RoomAmount를 모두 소모한 경우 Room 생성 종료, 구조물 생성
*/
void APGLevelGenerator::CheckOverlap(TObjectPtr<USceneComponent> InSelectedExitPoint, TObjectPtr<APGMasterRoom> RoomToCheck)
{	
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::CheckOverlap: Generation stopped"));
		if (RoomToCheck)
		{
			RoomToCheck->Destroy();
		}
		return;
	}

	if (IsLatestRoomOverlapping(RoomToCheck))
	{
		RoomToCheck->Destroy();
		SpawnNextRoom();
	}
	else
	{
		// add to room graph
		APGMasterRoom* ParentRoom = Cast<APGMasterRoom>(InSelectedExitPoint->GetOwner());
		if (ParentRoom)
		{
			RoomGraph.FindOrAdd(ParentRoom).Add(RoomToCheck);
			RoomGraph.FindOrAdd(RoomToCheck).Add(ParentRoom);
			bRoomDepthsDirty = true;
		}
		DoorPointsList.Add(InSelectedExitPoint);
		ExitPointsList.Remove(InSelectedExitPoint);

		RoomAmount--;

		RoomToCheck->SpawnPhotoSpots();
		RoomToCheck->SpawnSwingProps(Seed);

		// ExitPoints
		if (const USceneComponent* LatestRoomExitPointsFolder = RoomToCheck->GetExitPointsFolder())
		{
			const TArray<USceneComponent*>& LatestRoomExitPoints = LatestRoomExitPointsFolder->GetAttachChildren();
			ExitPointsList.Reserve(ExitPointsList.Num() + LatestRoomExitPoints.Num());
			ExitPointsList.Append(LatestRoomExitPoints);
		}

		//Searchable Points
		if (const USceneComponent* SearchableSpawnPointsFolder = RoomToCheck->GetSearchableSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& SearchableSpawnPoints = SearchableSpawnPointsFolder->GetAttachChildren();
			SearchableSpawnPointsList.Reserve(SearchableSpawnPointsList.Num() + SearchableSpawnPoints.Num());
			SearchableSpawnPointsList.Append(SearchableSpawnPoints);
		}

		// MannequinPoints
		if (const USceneComponent* MannequinSpawnPointFolder = RoomToCheck->GetMannequinSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& MannequinSpawnPoints = MannequinSpawnPointFolder->GetAttachChildren();
			MannequinSpawnPointsList.Reserve(MannequinSpawnPointsList.Num() + MannequinSpawnPoints.Num());
			MannequinSpawnPointsList.Append(MannequinSpawnPoints);
		}

		// ArmorStandPoints
		if (const USceneComponent* ArmorStandSpawnPointFolder = RoomToCheck->GetArmorStandSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& ArmorStandSpawnPoints = ArmorStandSpawnPointFolder->GetAttachChildren();
			ArmorStandSpawnPointsList.Reserve(ArmorStandSpawnPointsList.Num() + ArmorStandSpawnPoints.Num());
			ArmorStandSpawnPointsList.Append(ArmorStandSpawnPoints);
		}

		// FuseBox points
		if (const USceneComponent* FuseBoxSpawnPointFolder = RoomToCheck->GetFuseBoxSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& FuseBoxSpawnPoints = FuseBoxSpawnPointFolder->GetAttachChildren();
			FuseBoxSpawnPointsList.Reserve(FuseBoxSpawnPointsList.Num() + FuseBoxSpawnPoints.Num());
			FuseBoxSpawnPointsList.Append(FuseBoxSpawnPoints);
		}

		// Props(Hide prop, waiter stand) points 
		// 방 별로 포인트를 하나씩 가져오고, 그 포인트를 제거 후 나머지 포인터들 HideProp생성용으로 가져옴.
		AddPropsSpawnPoint(RoomToCheck);

		// Glass bottle spawn points
		if (const USceneComponent* GlassBottleSpawnPointFolder = RoomToCheck->GetGlassBottleSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& GlassBottleSpawnPoints = GlassBottleSpawnPointFolder->GetAttachChildren();
			GlassBottleSpawnPointsList.Reserve(GlassBottleSpawnPointsList.Num() + GlassBottleSpawnPoints.Num());
			GlassBottleSpawnPointsList.Append(GlassBottleSpawnPoints);
		}

		// Deco spawn
		if (const USceneComponent* BloodstainSpawnPointFolder = RoomToCheck->GetBloodstainSpawnPointsFolder())
		{
			const TArray<USceneComponent*>& BloodSpawnPoints = BloodstainSpawnPointFolder->GetAttachChildren();
			BloodstainSpawnPointsList.Reserve(BloodstainSpawnPointsList.Num() + BloodSpawnPoints.Num());
			for (USceneComponent* SpawnPoint : BloodSpawnPoints)
			{
				if (UPGBloodstainSpawnPoint* Point = Cast<UPGBloodstainSpawnPoint>(SpawnPoint))
				{
					BloodstainSpawnPointsList.Add(Point);
				}
			}
		}

		if (RoomAmount > 0)
		{
			SpawnNextRoom();
		}
		else
		{
			SetupLevelEnvironment();
		}
	}
}

/*
* Overlap 검사 구현부
* 생성된 Room의 OverlapBox와 충돌하는 오브젝트 확인
*/
bool APGLevelGenerator::IsLatestRoomOverlapping(const APGMasterRoom* RoomToCheck) const
{
	UWorld* World = GetWorld();
	if (!World || !RoomToCheck)
	{
		return false;
	}

	if (const USceneComponent* OverlapFolder = RoomToCheck->GetOverlapBoxFolder())
	{
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(RoomToCheck);

		for (const USceneComponent* SceneComp : OverlapFolder->GetAttachChildren())
		{
			if (const UBoxComponent* BoxComp = Cast<UBoxComponent>(SceneComp))
			{
				const bool bIsOverlapping = World->OverlapAnyTestByObjectType(
					BoxComp->GetComponentLocation(),
					BoxComp->GetComponentQuat(),
					FCollisionObjectQueryParams(ECollisionChannel::ECC_GameTraceChannel1),
					BoxComp->GetCollisionShape(),
					QueryParams
				);

				if (bIsOverlapping)
				{
					return true;
				}
			}
		}
	}

	return false;
}

/*
* Room 생성 종료 후 구조물 생성
* 벽, 문, 아이템 스폰
* 적대 AI 스폰
* 레벨 생성 작업 완료 후 GameState의 레벨 생성 완료 델리게이트 트리거
* 변수 초기화
*/
void APGLevelGenerator::SetupLevelEnvironment()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Generation stopped by Timeout."));
		return;
	}
	bIsGenerationStopped = true;
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	EnsureRoomDepthMap();

	CloseHoles();
	SpawnDoors();
	SpawnSearchables();
	SpawnItems();
	//SpawnMannequins();
	SpawnArmorStands();
	SpawnFuseBoxes();
	SpawnWaiterStands();
	SpawnHideProps();
	SpawnBloodStains();
	if (!SpawnEnemy())
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SetupLevelEnvironment: Enemy spawn failed. Restarting Level..."));
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		FTimerHandle TravelStartTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TravelStartTimerHandle,
			this,
			&APGLevelGenerator::ReGenerateLevel,
			1.0f,
			false
		);

		return;
	}
	ComputeExplorationWaypoints();

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->NotifyMapGenerationComplete();
	}

	ExitPointsList.Empty();
	DoorPointsList.Empty();
	MannequinSpawnPointsList.Empty();
	FuseBoxSpawnPointsList.Empty();
	BloodstainSpawnPointsList.Empty();
	RoomGraph.Empty();
	RoomDepths.Empty();
	RoomParents.Empty();
	SearchableOwnerRooms.Empty();
	bRoomDepthsDirty = true;
}

/*
* 모든 Room 생성 후 Room과 Room이 연결되지 않은 부분(뚫려있는 부분)에 벽 생성
*/
void APGLevelGenerator::CloseHoles()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (USceneComponent* ExitPoint : ExitPointsList)
	{
		if (!ExitPoint)
		{
			continue;
		}

		const FVector SpawnLocation = ExitPoint->GetComponentLocation();
		FRotator SpawnRotation = ExitPoint->GetComponentRotation();
		SpawnRotation.Yaw += 90.0f;
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGMasterRoom* OwnerRoom = Cast<APGMasterRoom>(ExitPoint->GetOwner());
		if (OwnerRoom && OwnerRoom->GetWallClass())
		{
			TSubclassOf<AActor> WallClass = OwnerRoom->GetWallClass();
			World->SpawnActor<AActor>(WallClass, SpawnTransform, SpawnParams);
		}
		else
		{
			World->SpawnActor<APGWall>(APGWall::StaticClass(), SpawnTransform, SpawnParams);
		}
	}
}

/*
* 모든 Room 생성 후 Room과 Room이 연결된 부분(DoorPointsList)에 문 생성
* 일부는 문이 없거나 잠긴 문 생성
*/
void APGLevelGenerator::SpawnDoors()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 DoorAmount = DoorPointsList.Num() * 0.8f;
	int32 LockedDoorAmount = DoorAmount * 0.3f;

	while (DoorAmount > 0 && !DoorPointsList.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, DoorPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedDoorPoint = DoorPointsList[RandomIndex];
		DoorPointsList.RemoveAt(RandomIndex);
		if (!SelectedDoorPoint)
		{
			continue;
		}

		const FVector SpawnLocation = SelectedDoorPoint->GetComponentLocation();
		const FRotator SpawnRotation = SelectedDoorPoint->GetComponentRotation();
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;

		// const bool bShouldBeLocked = (LockedDoorAmount > 0);
		const bool bShouldBeLocked = false;

		check(PGDoor);
		APGDoor1* DefaultDoor = PGDoor->GetDefaultObject<APGDoor1>();
		if (DefaultDoor)
		{
			DefaultDoor->SpawnDoor(World, PGDoor, SpawnTransform, SpawnParams, bShouldBeLocked);
		}

		if (bShouldBeLocked)
		{
			LockedDoorAmount--;
		}
		DoorAmount--;
	}
}

void APGLevelGenerator::SpawnSearchables()
{
	UWorld* World = GetWorld();
	if (!World || SearchableSpawnPointsList.IsEmpty())
	{
		return;
	}

	for (TObjectPtr<USceneComponent> Point : SearchableSpawnPointsList)
	{
		UPGSearchableSpawnPoint* SearchablePoint = Cast<UPGSearchableSpawnPoint>(Point);
		if (!SearchablePoint)
		{
			continue; 
		}
		APGMasterRoom* OwnerRoom = Cast<APGMasterRoom>(SearchablePoint->GetOwner());

		ESearchableType TypeToSpawn = SearchablePoint->GetSearchableType();
		// TMap에서 해당 Enum에 매핑된 클래스 포인터 찾기
		if (const TSubclassOf<APGSearchableBase>* ClassPtr = SearchableClassMap.Find(TypeToSpawn))
		{
			// 클래스가 비어있지 않은지 검증
			if (*ClassPtr)
			{
				const FTransform SpawnTransform = SearchablePoint->GetComponentTransform();
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APGSearchableBase* NewSearchable = World->SpawnActor<APGSearchableBase>(*ClassPtr, SpawnTransform, SpawnParams);
				if (NewSearchable)
				{
					SpawnedSearchables.Add(NewSearchable);
					SearchableOwnerRooms.Add(NewSearchable, OwnerRoom);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("LG::SpawnSearchables: Map contains valid Enum but class is empty!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LG::SpawnSearchables: Enum type not found in SearchableClassMap!"));
		}
	}
	UE_LOG(LogTemp, Log, TEXT("SpawnSearchables Num : %d"), SpawnedSearchables.Num());
}

void APGLevelGenerator::AddPropsSpawnPoint(TObjectPtr<APGMasterRoom> RoomToCheck)
{
	if (const USceneComponent* PropsSpawnPointsFolder = RoomToCheck->GetPropsSpawnPointsFolder())
	{
		// 1. 자식 컴포넌트(스폰 포인트들) 배열 가져오기
		const TArray<USceneComponent*>& PropsSpawnPoints = PropsSpawnPointsFolder->GetAttachChildren();

		if (PropsSpawnPoints.Num() == 0)
		{
			return;
		}

		// Points들을 랜덤하게 섞는다.
		TArray<USceneComponent*> ShuffledPoints = PropsSpawnPoints;
		for (int32 i = ShuffledPoints.Num() - 1; i > 0; i--)
		{
			int32 j = FMath::RandRange(0, i);
			ShuffledPoints.Swap(i, j);
		}

		//  WaiterStandSpawnPointsList로 1개 고정
		WaiterStandSpawnPointsList.Add(ShuffledPoints[0]);

		// 나머지는 HidePropSpawnPointsList로 ( 총 5개이므로, 1~4 4개가 들어간다 )
		for (int32 i = 1; i < ShuffledPoints.Num(); ++i)
		{
			HidePropSpawnPointsList.Add(ShuffledPoints[i]);
		}
	}
}

/*
* 모든 Room 생성 후 아이템 스폰
* 재귀를 통한 비동기 아이템 로드/스폰
*/
void APGLevelGenerator::SpawnItems()
{
	// Spawn exit items
	SpawnExitItems();

	// Spawn basic items
	// 스폰하기 전에 배열을 섞어서, 특정 아이템이 앞이나 뒤에 쏠리는 현상을 방지
	//Algo::RandomShuffle(SpawnedSearchables);
	for (int32 i = SpawnedSearchables.Num() - 1; i > 0; --i)
	{
		const int32 j = Seed.RandRange(0, i);
		SpawnedSearchables.Swap(i, j);
	}
	const int32 ItemAmount = 16;
	SpawnSingleItem_Async(ItemAmount, 0);

	// Spawn glass bottles
	for (TObjectPtr<USceneComponent> SpawnPoint : GlassBottleSpawnPointsList)
	{
		// 20% 확률로 생성 안함
		if (Seed.FRand() < 0.2f)
		{
			continue;
		}

		if (SpawnPoint)
		{
			UWorld* World = GetWorld();
			UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
			if (!GI || !World)
			{
				return;
			}

			const FTransform SpawnTransform(SpawnPoint->GetComponentTransform());

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			TWeakObjectPtr<UWorld> WeakWorld = World;

			GI->RequestLoadItemData(FName("GlassBottle"), FOnItemDataLoaded::CreateLambda(
				[WeakWorld, SpawnTransform, SpawnParams](UPGItemData* LoadedItemData)
				{
					if (!LoadedItemData || !WeakWorld.IsValid())
					{
						return;
					}

					APGItemActor* NewItem = WeakWorld->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
					if (!NewItem)
					{
						 return;
					}

					NewItem->InitWithData(LoadedItemData);
				}));
		}
	}
}

void APGLevelGenerator::SpawnSingleItem_Async(int32 ItemAmount, int32 SeqIndex)
{
	UWorld* World = GetWorld();

	// 종료 조건: 스폰할 아이템이 없거나, 배치할 Searchable이 남지 않은 경우
	if (!World || ItemAmount <= 0 || SpawnedSearchables.IsEmpty())
	{
		SpawnedSearchables.Empty();
		UE_LOG(LogTemp, Log, TEXT("PGLevelGenerator : Spawning Finished."));
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI) return;

	FName ItemKeyToLoad;

	// ItemAmount에 맞춰서 아이템 스폰
	if (ItemAmount > 12) ItemKeyToLoad = FName("ReviveKit");
	else if (ItemAmount > 5) ItemKeyToLoad = FName("Match");
	else ItemKeyToLoad = FName("GlassBottle");

	// 순차 탐색 (1단계) vs 랜덤 탐색 (2단계) 인덱스 결정
	// 처음에는 하나씩 아이템을 넣고, 이제 남은 아이템은 랜덤하게 결정

	int32 SelectedSearchableIndex = -1;
	bool bIsSequentialPhase = (SeqIndex < SpawnedSearchables.Num());

	if (bIsSequentialPhase)
	{
		// 전체 배열을 1회 순회하기 전이라면 순차적으로 접근
		SelectedSearchableIndex = SeqIndex;
	}
	else
	{
		// 전체 배열 순회를 마쳤다면 무작위로 하나 선택
		SelectedSearchableIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, SpawnedSearchables.Num());
	}

	APGSearchableBase* SelectedSearchable = SpawnedSearchables[SelectedSearchableIndex];
	APGSearchableSlotBase* SelectedSlot = nullptr;

	bool bHasMoreSlots = false;
	if (IsValid(SelectedSearchable))
	{
		// 슬롯 하나 꺼내기. 더 이상 남은 슬롯이 없으면 False 반환됨
		bHasMoreSlots = SelectedSearchable->GetRandomSlot(SelectedSlot, Seed);
	}

	// 예외 처리: 유효한 슬롯을 가져오지 못했다면 해당 가구를 제거하고 다시 시도
	if (!SelectedSlot)
	{
		SpawnedSearchables.RemoveAt(SelectedSearchableIndex);
		SpawnSingleItem_Async(ItemAmount, SeqIndex); // ItemAmount 차감 없이 재귀 호출
		return;
	}

	// 다음 비동기 호출로 넘겨줄 인덱스 계산
	int32 NextSeqIndex = SeqIndex;

	if (!bHasMoreSlots)
	{
		// False가 리턴되었다면 더 이상 남은 자리가 없으므로 배열에서 완전히 제거
		SpawnedSearchables.RemoveAt(SelectedSearchableIndex);

		// 순차 탐색 중에 요소를 제거했다면 뒤의 배열 요소들이 당겨지므로,
		// NextSeqIndex를 증가시키지 않아야 다음 요소를 정상적으로 탐색할 수 있음.
	}
	else
	{
		// 요소를 제거하지 않았고, 순차 탐색 단계였다면 다음 인덱스로 넘어감
		if (bIsSequentialPhase && SelectedSearchableIndex == SeqIndex)
		{
			NextSeqIndex = SeqIndex + 1;
		}
	}

	ItemAmount--;

	// 스폰 Transform 세팅
	USceneComponent* ItemSpawnComp = SelectedSlot->GetItemSpawnPoint();
	FVector SpawnLocation = ItemSpawnComp ? ItemSpawnComp->GetComponentLocation() : SelectedSlot->GetActorLocation();
	FRotator SpawnRoation = ItemSpawnComp ? ItemSpawnComp->GetComponentRotation() : SelectedSlot->GetActorRotation();
	const FTransform SpawnTransform(SpawnRoation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TWeakObjectPtr<APGLevelGenerator> WeakThis(this);
	TWeakObjectPtr<UWorld> WeakWorld = World;
	TWeakObjectPtr<APGSearchableSlotBase> WeakSlot = SelectedSlot;

	GI->RequestLoadItemData(ItemKeyToLoad, FOnItemDataLoaded::CreateLambda(
		[WeakThis, WeakWorld, WeakSlot, SpawnTransform, SpawnParams, ItemAmount, NextSeqIndex](UPGItemData* LoadedItemData)
		{
			if (!LoadedItemData || !WeakThis.IsValid() || !WeakWorld.IsValid())
			{
				return;
			}

			APGItemActor* NewItem = WeakWorld->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
			if (!NewItem)
			{
				return;
			}

			NewItem->InitWithData(LoadedItemData);

			// 아이템을 슬롯에 장착시킴. 슬롯과 같이 움직이도록.
			if (WeakSlot.IsValid())
			{
				// 아이템을 장착할 때, 슬롯에 맞게 회전시키는 경우들 확인
				FGameplayTagContainer RotateItemTags;
				RotateItemTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.RustOil")));
				RotateItemTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.GlassBottle")));

				bool bSpawnItemWithRotate = RotateItemTags.HasTagExact(LoadedItemData->ItemTag);
				WeakSlot->AttachSpawnedItem(NewItem, bSpawnItemWithRotate);
			}

			// 재귀 호출 (증가되거나 보정된 인덱스를 넘겨줌)
			WeakThis->SpawnSingleItem_Async(ItemAmount, NextSeqIndex);
		}));
}

/*
* 탈출 핵심 아이템(ChainKey / HandWheel / RustOil) 스폰
* RoomGraph 깊이 절반+1 이상인 방에, 서로 다른 방/다른 Searchable에 스폰
* 일반 아이템보다 먼저 호출되어 슬롯 선점
*/
void APGLevelGenerator::SpawnExitItems()
{
	EnsureRoomDepthMap();

	int32 MaxDepth = 0;
	for (const auto& Elem : RoomDepths)
	{
		MaxDepth = FMath::Max(MaxDepth, Elem.Value);
	}

	const int32 MinDepth = FMath::Max(1, MaxDepth / 2);
	TSet<TObjectPtr<APGMasterRoom>> UsedRooms;
	TSet<TObjectPtr<APGMasterRoom>> UsedBranches;
	TSet<TObjectPtr<APGSearchableBase>> UsedSearchables;
	for (const FName& ItemKey : ExitItemKeys)
	{
		APGSearchableSlotBase* Slot = AcquireExitItemSlot(MinDepth, UsedRooms, UsedBranches, UsedSearchables);
		if (!Slot)
		{
			UE_LOG(LogTemp, Error, TEXT("LG::SpawnExitItems: Failed to find slot for %s"), *ItemKey.ToString());
			continue;
		}

		SpawnItemAtSlot(ItemKey, Slot);
	}

	// Branch 수 = StartRoom 인접 방 수
	const int32 BranchCount = RoomGraph.Contains(StartRoom) ? RoomGraph[StartRoom].Num() : 0;

	UE_LOG(LogTemp, Log, TEXT("LG::SpawnExitItems: MaxDepth=%d MinDepth=%d Branches=%d / UsedBranches=%d UsedRooms=%d"),
		MaxDepth, MinDepth, BranchCount, UsedBranches.Num(), UsedRooms.Num());
}

void APGLevelGenerator::SpawnItemAtSlot(const FName& ItemKey, APGSearchableSlotBase* Slot)
{
	UWorld* World = GetWorld();
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!World || !GI || !Slot)
	{
		return;
	}

	USceneComponent* ItemSpawnComp = Slot->GetItemSpawnPoint();
	const FVector SpawnLocation = ItemSpawnComp ? ItemSpawnComp->GetComponentLocation() : Slot->GetActorLocation();
	const FRotator SpawnRotation = ItemSpawnComp ? ItemSpawnComp->GetComponentRotation() : Slot->GetActorRotation();
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TWeakObjectPtr<UWorld> WeakWorld = World;
	TWeakObjectPtr<APGSearchableSlotBase> WeakSlot = Slot;

	GI->RequestLoadItemData(ItemKey, FOnItemDataLoaded::CreateLambda([WeakWorld, SpawnTransform, SpawnParams, WeakSlot](UPGItemData* LoadedItemData)
	{
		if (!LoadedItemData || !WeakWorld.IsValid())
		{
			return;
		}

		APGItemActor* NewItem = WeakWorld->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
		if (!NewItem)
		{
			return;
		}

		NewItem->InitWithData(LoadedItemData);

		if (WeakSlot.IsValid())
		{
			FGameplayTagContainer RotateItemTags;
			RotateItemTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.RustOil")));
			RotateItemTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.GlassBottle")));

			const bool bSpawnItemWithRotate = RotateItemTags.HasTagExact(LoadedItemData->ItemTag);
			WeakSlot->AttachSpawnedItem(NewItem, bSpawnItemWithRotate);
		}
	}));
}

/*
* Pass 0: Depth + 서로 다른 Branch	(Branch, Room 중복 x)
* Pass 1: Depth + 서로 다른 Room		(Branch 중복 허용)
* Pass 2: Depth						(Room 중복 허용)
* Pass 3: 제한 없음					(Branch, Room 중복 허용 + Room 최소 Depth x)
* Searchable 중복 금지는 항상 적용
*/
APGSearchableSlotBase* APGLevelGenerator::AcquireExitItemSlot(
	int32 MinDepth,
	TSet<TObjectPtr<APGMasterRoom>>& UsedRooms,
	TSet<TObjectPtr<APGMasterRoom>>& UsedBranches,
	TSet<TObjectPtr<APGSearchableBase>>& UsedSearchables)
{
	// 0->1->2->3 순서대로 제한 완화하며 스폰할 SearchableSlot return
	for (int32 Pass = 0; Pass < 4; ++Pass)
	{
		const bool bRequireDepth = (Pass <= 2);
		const bool bRequireUnusedBranch = (Pass == 0);
		const bool bRequireUnusedRoom = (Pass <= 1);

		TArray<TObjectPtr<APGSearchableBase>> Candidates;
		for (const TObjectPtr<APGSearchableBase>& Searchable : SpawnedSearchables)
		{
			if (!IsValid(Searchable) || UsedSearchables.Contains(Searchable))
			{
				// 이미 쓴 Searchable이면 continue. Searchable은 무조건 중복 금지
				continue;
			}

			const TObjectPtr<APGMasterRoom>* RoomPtr = SearchableOwnerRooms.Find(Searchable); // 판정중인 Searchable이 있는 Room
			APGMasterRoom* Room = RoomPtr ? RoomPtr->Get() : nullptr;
			const int32* DepthPtr = Room ? RoomDepths.Find(Room) : nullptr; // 그 Room의 Depth(StartRoom부터의 거리)
			if (!DepthPtr || *DepthPtr <= 0)
			{
				continue;
			}

			if (bRequireDepth && *DepthPtr < MinDepth)
			{
				// Depth 체크 하는 단계(0~2)인 경우 Depth 제한 못넘으면 continue
				continue;
			}

			if (bRequireUnusedRoom && UsedRooms.Contains(Room))
			{
				// Room 중복 체크 하는 단계(0~1)인 경우 Room 중복이면 continue
				continue;
			}

			if (bRequireUnusedBranch)
			{
				// Branch 중복 체크 하는 단계(0)인 경우 Branch 중복이면 continue
				APGMasterRoom* BranchRoot = GetBranchRoot(Room);
				if (!BranchRoot || UsedBranches.Contains(BranchRoot))
				{
					continue;
				}
			}

			Candidates.Add(Searchable);
		}

		// 해당 단계에서 모인 Searchable Candidates에서 뽑기
		while (!Candidates.IsEmpty())
		{
			const int32 Pick = UKismetMathLibrary::RandomIntegerFromStream(Seed, Candidates.Num());
			TObjectPtr<APGSearchableBase> Searchable = Candidates[Pick];
			Candidates.RemoveAtSwap(Pick);

			APGSearchableSlotBase* OutSlot = nullptr;
			const bool bHasMoreSlots = Searchable->GetRandomSlot(OutSlot, Seed);

			if (!bHasMoreSlots)
			{
				SpawnedSearchables.Remove(Searchable);
			}

			// 선택된 Searchable, Searchable이 있는 Room, Branch 사용 목록에 등록
			if (OutSlot)
			{
				UsedSearchables.Add(Searchable);

				if (const TObjectPtr<APGMasterRoom>* RoomPtr = SearchableOwnerRooms.Find(Searchable))
				{
					UsedRooms.Add(*RoomPtr);

					if (APGMasterRoom* BranchRoot = GetBranchRoot(RoomPtr->Get()))
					{
						UsedBranches.Add(BranchRoot);
					}
				}

				UE_LOG(LogTemp, Log, TEXT("LG::AcquireExitItemSlot: Pass=%d Depth=%d"), Pass, RoomDepths.FindRef(SearchableOwnerRooms.FindRef(Searchable)));

				return OutSlot;
			}
		}
	}

	return nullptr;
}

/*
* Input Room's Branch return
*/
APGMasterRoom* APGLevelGenerator::GetBranchRoot(APGMasterRoom* Room) const
{
	while (Room)
	{
		const int32* DepthPtr = RoomDepths.Find(Room);
		if (!DepthPtr || *DepthPtr <= 0)
		{
			return nullptr;
		}

		if (*DepthPtr == 1)
		{
			return Room;
		}

		Room = RoomParents.FindRef(Room);
	}

	return nullptr;
}

TObjectPtr<USceneComponent> APGLevelGenerator::GetRandomPointFromSpecificListAndRemove(TArray<TObjectPtr<USceneComponent>>& TargetList, TArray<TObjectPtr<USceneComponent>>& TargetRemoveList)
{
	if (TargetList.IsEmpty())
	{
		return nullptr;
	}

	const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, TargetList.Num());
	TObjectPtr<USceneComponent> Point = TargetList[RandomIndex];

	// 두 리스트 모두에서 제거
	TargetList.RemoveAt(RandomIndex);
	TargetRemoveList.Remove(Point);

	return Point;
}

void APGLevelGenerator::SpawnMannequins()
{
	UWorld* World = GetWorld();
	if (!World || MannequinSpawnPointsList.IsEmpty())
	{
		return;
	}

	int32 MannequinAmount = MannequinSpawnPointsList.Num() * 0.4f;

	while (MannequinAmount > 0 && !MannequinSpawnPointsList.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MannequinSpawnPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedSpawnPoint = MannequinSpawnPointsList[RandomIndex];
		MannequinSpawnPointsList.RemoveAt(RandomIndex);

		if (SelectedSpawnPoint)
		{
			const FTransform SpawnTransform(SelectedSpawnPoint->GetComponentTransform());
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			World->SpawnActor<APGTriggerGimmickMannequin>(MannequinClass, SpawnTransform, SpawnParams);
			MannequinAmount--;
		}
	}
}

void APGLevelGenerator::SpawnArmorStands()
{
	UE_LOG(LogTemp, Log, TEXT("Spawn Armor Stand function"));
	UWorld* World = GetWorld();
	if (!World || ArmorStandSpawnPointsList.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot Spawn Armor Stand"));
		return;
	}

	for (TObjectPtr<USceneComponent> SelectedPoint : ArmorStandSpawnPointsList)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawn Armor Stand"));
		const FTransform SpawnTransform(SelectedPoint->GetComponentTransform());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		World->SpawnActor<APGInteractableGimmickArmorStand>(ArmorStandClass, SpawnTransform, SpawnParams);
	}
}

void APGLevelGenerator::SpawnFuseBoxes()
{
	UWorld* World = GetWorld();
	if (!World || FuseBoxSpawnPointsList.IsEmpty() || !FuseBoxClass)
	{
		return;
	}

	int32 FuseBoxCount = FMath::Min(2, FuseBoxSpawnPointsList.Num());
	while (FuseBoxCount > 0 && !FuseBoxSpawnPointsList.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FuseBoxSpawnPointsList.Num());
		const TObjectPtr<USceneComponent> SelectedPoint = FuseBoxSpawnPointsList[RandomIndex];
		FuseBoxSpawnPointsList.RemoveAt(RandomIndex);

		if (!SelectedPoint)
		{
			continue;
		}

		const FTransform SpawnTransform = SelectedPoint->GetComponentTransform();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGFuseBox* NewFuseBox = World->SpawnActor<APGFuseBox>(FuseBoxClass, SpawnTransform, SpawnParams);
		if (NewFuseBox)
		{
			AActor* OwnerRoom = SelectedPoint->GetOwner();
			NewFuseBox->SetOwnerRoom(OwnerRoom);

			UE_LOG(LogTemp, Log, TEXT("LG::SpawnFuseBoxes: Spawned FuseBox in room '%s'"), *GetNameSafe(OwnerRoom));
		}

		FuseBoxCount--;
	}
}

void APGLevelGenerator::SpawnWaiterStands()
{
	UWorld* World = GetWorld();
	if (!World || WaiterStandSpawnPointsList.IsEmpty() || !WaiterStandClass)
	{
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		int32 RandomIndex = FMath::RandRange(0, WaiterStandSpawnPointsList.Num() - 1);
		const TObjectPtr<USceneComponent> SelectedPoint = WaiterStandSpawnPointsList[RandomIndex];
		WaiterStandSpawnPointsList.RemoveAt(RandomIndex);

		if (!SelectedPoint)
		{
			continue;
		}

		const FTransform SpawnTransform = SelectedPoint->GetComponentTransform();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGWaiterStand* NewWaiterStand = World->SpawnActor<APGWaiterStand>(WaiterStandClass, SpawnTransform, SpawnParams);
		if (NewWaiterStand)
		{
			int32 BottleCount = FMath::RandRange(3, 6);

			NewWaiterStand->SpawnItems(BottleCount);

			UE_LOG(LogTemp, Log, TEXT("LGLevelGenerator::SpawnWaiterStand: Spawned WaiterStand with %d bottles"), BottleCount);
		}
	}
}

void APGLevelGenerator::SpawnHideProps()
{
	const int32 PropsNum = HidePropClasses.Num();
	int32 CurProp = 0;

	// 방 인덱스 리스트 생성 (0~4번 방) 후 섞기
	TArray<int32> RoomIndices;
	for (int32 i = 0; i < HidePropSpawnPointsList.Num() / 4; ++i)
	{
		RoomIndices.Add(i);
	}

	for (int32 i = RoomIndices.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		RoomIndices.Swap(i, j);
	}

	// 반은 2개씩, 나머지 반은 1개씩 생성
	for (int32 i = 0; i < RoomIndices.Num() ; ++i)
	{
		int32 SelectedRoomIdx = RoomIndices[i];

		// 해당 방의 시작 인덱스 계산 (방당 4개씩)
		int32 StartIdx = SelectedRoomIdx * 4;
		int32 EndIdx = StartIdx + 3;

		// 방 내부의 4개 포인트 중 2개를 랜덤 선택
		TArray<int32> Points;
		for (int32 j = StartIdx; j <= EndIdx; ++j)
		{
			Points.Add(j);
		}

		for (int32 j = Points.Num() - 1; j > 0; --j)
		{
			int32 k = FMath::RandRange(0, j);
			Points.Swap(j, k);
		}

		int32 RandomPointIdx = Points[0];
		int32 RandomPointIdx2 = Points[1];

		// Spawn
		if (HidePropSpawnPointsList.IsValidIndex(RandomPointIdx) && HidePropSpawnPointsList[RandomPointIdx])
		{
			USceneComponent* SpawnPoint = HidePropSpawnPointsList[RandomPointIdx];
			FTransform SpawnTransform = SpawnPoint->GetComponentTransform();

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			APGHideProp* NewProp = GetWorld()->SpawnActor<APGHideProp>(
				HidePropClasses[CurProp], // 한 종류씩 순서대로 꺼냄
				SpawnTransform,
				Params
			);

			UE_LOG(LogTemp, Log, TEXT("Spawn HideProp. PropIdx[%d] SpawnPointIndex[%d] "), CurProp, RandomPointIdx);

			const int32 RandPropChoose = FMath::RandRange(1, PropsNum - 1);
			CurProp += RandPropChoose;
			CurProp %= PropsNum;
		}

		// 몇몇 방은 2개 Spawn
		if (i < RoomIndices.Num() / 2)
		{
			if (HidePropSpawnPointsList.IsValidIndex(RandomPointIdx2) && HidePropSpawnPointsList[RandomPointIdx2])
			{
				USceneComponent* SpawnPoint = HidePropSpawnPointsList[RandomPointIdx2];
				FTransform SpawnTransform = SpawnPoint->GetComponentTransform();

				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				APGHideProp* NewProp = GetWorld()->SpawnActor<APGHideProp>(
					HidePropClasses[CurProp],
					SpawnTransform,
					Params
				);

				UE_LOG(LogTemp, Log, TEXT("Spawn HideProp. PropIdx[%d] SpawnPointIndex[%d] "), CurProp, RandomPointIdx2);

				const int32 RandPropChoose = FMath::RandRange(1, PropsNum - 1);
				CurProp += RandPropChoose;
				CurProp %= PropsNum;
			}
		}
	}
}

void APGLevelGenerator::SpawnBloodStains()
{
	UWorld* World = GetWorld();
	if (!World || BloodstainSpawnPointsList.IsEmpty())
	{
		return;
	}

	float Intensity = 0.5f;
	if (APGGameState* GS = World->GetGameState<APGGameState>())
	{
		Intensity = GS->GetDifficulty().HorrorDecorIntensity;
	}

	for (UPGBloodstainSpawnPoint* Point : BloodstainSpawnPointsList)
	{
		if (!Point || !Point->DecalClass)
		{
			continue;
		}

		if (UKismetMathLibrary::RandomFloatFromStream(Seed) > Intensity)
		{
			continue;
		}

		const FTransform SpawnTransform = Point->GetComponentTransform();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APGBloodstainDecal* SpawnedDecal = World->SpawnActor<APGBloodstainDecal>(Point->DecalClass, SpawnTransform, SpawnParams);
		if (SpawnedDecal)
		{
			SpawnedDecal->InitializeDecal(Point->DecalSize, Point->DecalMaterial);
		}
	}
}

/*
* 모든 Room 생성 후 적대 AI 스폰
* StartRoom 기준 중간 거리의 Room에 적대 AI 스폰
*/
bool APGLevelGenerator::SpawnEnemy()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APGGameMode* GM = World->GetAuthGameMode<APGGameMode>();
	if (!GM)
	{
		return false;
	}

	//APGMasterRoom* EnemySpawnRoom = FindFarthestRoom();
	const APGMasterRoom* BlindSpawnRoom = FindFarthestRoom();
	if (BlindSpawnRoom)
	{
		const FTransform SpawnTransform(FRotator::ZeroRotator, BlindSpawnRoom->GetEnemySpawnLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGBlindCharacter* SpawnedBlindCharacter = World->SpawnActor<APGBlindCharacter>(BlindCharacter, SpawnTransform, SpawnParams);
		if (SpawnedBlindCharacter)
		{
			SpawnedBlindCharacter->InitSoundManager(GM->GetSoundManager());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn blind"));
			return false;
		}

		UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn enemy at room '%s'"), *BlindSpawnRoom->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn blind"));
		return false;
	}

	const APGMasterRoom* ChargerSpawnRoom = FindMiddleDistanceRoom(BlindSpawnRoom->GetEnemySpawnLocation());
	if (ChargerSpawnRoom)
	{
		const FTransform ChargerSpawnTransform(FRotator::ZeroRotator, ChargerSpawnRoom->GetEnemySpawnLocation());
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APGChargerCharacter* SpawnedCharger = World->SpawnActor<APGChargerCharacter>(ChargerCharacter, ChargerSpawnTransform, SpawnParams);
		if (SpawnedCharger)
		{
			SpawnedCharger->InitSoundManager(GM->GetSoundManager());
			UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn Charger at room '%s'"), *ChargerSpawnRoom->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn charger"));
			return false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn charger"));
		return false;
	}

	const APGMasterRoom* GhostSpawnRoom = FindFarthestRoom();
	if (GhostSpawnRoom)
	{
		const FTransform SpawnTransform(FRotator::ZeroRotator, GhostSpawnRoom->GetEnemySpawnLocation());
		UE_LOG(LogTemp, Log, TEXT("LG::SpawnEnemy: Spawn ghost. (Room: %s, Location: %s)"), *GhostSpawnRoom->GetName(), *SpawnTransform.GetLocation().ToString());
		GM->SpawnGhost(SpawnTransform);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::SpawnEnemy: Failed to spawn ghost"));
		return false;
	}

	return true;
}

/*
* ExitPoint가 부족하여 더이상 Room을 생성할 수 없는 상황을 방지하기 위해 타이머 체크
* MaxGenerateTime(8초) 이후에도 레벨 생성 중이라면 재생성
*/
void APGLevelGenerator::StartLevelGenerateTimer()
{
	FTimerHandle LevelGenerateTimer;
	GetWorld()->GetTimerManager().SetTimer(
		LevelGenerateTimer,
		this,
		&APGLevelGenerator::CheckLevelGenerateTimeOut,
		1.0f,
		true,
		0.0f
	);
}

/*
* elapsed time 체크
*/
void APGLevelGenerator::CheckLevelGenerateTimeOut()
{
	if (bIsGenerationStopped)
	{
		UE_LOG(LogTemp, Warning, TEXT("LG::CheckLevelGenerateTimeOut: Level generation already successed"));
		return;
	}

	const float ElapsedTime = GetWorld()->GetTimeSeconds() - GenerationStartTime;
	if (ElapsedTime >= MaxGenerateTime)
	{
		bIsGenerationStopped = true;
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

		UE_LOG(LogTemp, Log, TEXT("LG::CheckLevelGenerateTimeout: Timeout. re-open level"));
		FTimerHandle TravelStartTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TravelStartTimerHandle,
			this,
			&APGLevelGenerator::ReGenerateLevel,
			1.0f,
			false
		);
	}
}

void APGLevelGenerator::ReGenerateLevel()
{
	UE_LOG(LogTemp, Log, TEXT("LG::ReGenerateLevel: ServerTravel."));
	GetWorld()->ServerTravel("/Game/ProjectG/Levels/LV_PGMainLevel?listen", true);
}

void APGLevelGenerator::EnsureRoomDepthMap()
{
	if (!bRoomDepthsDirty)
	{
		return;
	}

	BuildRoomDepthMap();
	bRoomDepthsDirty = false;
}

void APGLevelGenerator::BuildRoomDepthMap()
{
	RoomDepths.Empty();
	RoomParents.Empty();

	if (!StartRoom)
	{
		return;
	}

	RoomDepths.Add(StartRoom, 0);

	TQueue<TObjectPtr<APGMasterRoom>> Queue;
	Queue.Enqueue(StartRoom);
	while (!Queue.IsEmpty())
	{
		TObjectPtr<APGMasterRoom> Current;
		Queue.Dequeue(Current);

		const int32 CurrentDepth = RoomDepths[Current];

		if (const TArray<TObjectPtr<APGMasterRoom>>* Neighbors = RoomGraph.Find(Current))
		{
			for (const TObjectPtr<APGMasterRoom>& Neighbor : *Neighbors)
			{
				if (!Neighbor || RoomDepths.Contains(Neighbor))
				{
					continue;
				}

				RoomDepths.Add(Neighbor, CurrentDepth + 1);
				RoomParents.Add(Neighbor, Current);
				Queue.Enqueue(Neighbor);
			}
		}
	}
}

/*
* RoomGrpah에 대해 BFS 방식으로 StartRoom에서 가장 먼 Room 탐색
* 가장 먼 Room 중 랜덤 석택
*/
const APGMasterRoom* APGLevelGenerator::FindFarthestRoom() const
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	// find farthest room	
	TArray<TObjectPtr<APGMasterRoom>> FarthestRooms;
	int32 MaxDistance = -1;
	for (const auto& Elem : RoomDepths)
	{
		if (Elem.Value > MaxDistance)
		{
			MaxDistance = Elem.Value;

			FarthestRooms.Empty();
			FarthestRooms.Add(Elem.Key);
		}
		else if (Elem.Value == MaxDistance)
		{
			FarthestRooms.Add(Elem.Key);
		}
	}

	// random select
	if (!FarthestRooms.IsEmpty())
	{
		const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, FarthestRooms.Num());
		return FarthestRooms[RandomIndex];
	}	
		
	return nullptr;
}

/*
* RoomGrpah에 대해 BFS 방식으로 StartRoom에서 중간 거리 Room 탐색
* 중간 거리 Room 중 랜덤 선택
*/
const APGMasterRoom* APGLevelGenerator::FindMiddleDistanceRoom(const FVector& AvoidLocation) const
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		return nullptr;
	}

	int32 MaxDistance = 0;
	for (const auto& Elem : RoomDepths)
	{
		MaxDistance = FMath::Max(MaxDistance, Elem.Value);
	}

	// find middle distance room
	const int32 TargetDistance = MaxDistance / 2 + 1;
	TArray<TObjectPtr<APGMasterRoom>> MiddleDistanceRooms;
	for (const auto& Elem : RoomDepths)
	{
		if (Elem.Value == TargetDistance)
		{
			MiddleDistanceRooms.Add(Elem.Key);
		}
	}

	if (MiddleDistanceRooms.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[LG::FindMiddleDistanceRoom] no middle distance rooms"));
		return nullptr;
	}

	// if AvoidLocation != 0
	if (!AvoidLocation.IsZero())
	{
		TObjectPtr<APGMasterRoom> FarthestRoom = nullptr;
		float MaxDist = -1.0f;

		for (const TObjectPtr<APGMasterRoom>& Room : MiddleDistanceRooms)
		{
			float Dist = FVector::Dist(Room->GetEnemySpawnLocation(), AvoidLocation);
			if (Dist > MaxDist)
			{
				MaxDist = Dist;
				FarthestRoom = Room;
			}
		}
		return FarthestRoom;
	}

	// Default(AvoidLocation == 0): random select
	const int32 RandomIndex = UKismetMathLibrary::RandomIntegerFromStream(Seed, MiddleDistanceRooms.Num());
	return MiddleDistanceRooms[RandomIndex];
}

void APGLevelGenerator::ComputeExplorationWaypoints()
{
	if (!StartRoom || !RoomGraph.Contains(StartRoom))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LG] No valid StartRoom, RoomGraph"));
		return;
	}

	int32 MaxDistance = 0;
	for (const auto& Elem : RoomDepths)
	{
		MaxDistance = FMath::Max(MaxDistance, Elem.Value);
	}

	TArray<TObjectPtr<APGMasterRoom>> LeafRooms;
	for (const auto& Elem : RoomGraph)
	{
		if (Elem.Key == StartRoom)
		{
			continue;
		}

		if (Elem.Value.Num() <= 1)
		{
			LeafRooms.Add(Elem.Key);
		}
	}

	TArray<FVector> Waypoints;
	TSet<TObjectPtr<APGMasterRoom>> UsedRooms;

	for (const TObjectPtr<APGMasterRoom>& Leaf : LeafRooms)
	{
		int32 LeafDist = RoomDepths.Contains(Leaf) ? RoomDepths[Leaf] : 0;
		int32 TargetDist = FMath::RoundToInt(LeafDist * 0.75f);

		TObjectPtr<APGMasterRoom> Current = Leaf;
		while (Current && RoomDepths.Contains(Current))
		{
			if (RoomDepths[Current] == TargetDist)
			{
				if (!UsedRooms.Contains(Current))
				{
					UsedRooms.Add(Current);
					Waypoints.Add(Current->GetEnemySpawnLocation());
				}
				break;
			}

			if (RoomParents.Contains(Current))
			{
				Current = RoomParents[Current];
			}
			else
			{
				break;
			}
		}
	}

	if (Waypoints.Num() < 4)
	{
		UE_LOG(LogTemp, Log, TEXT("LG::ComputeExplorationWaypoints: Too little waypoints"));
		const int32 MidDist = MaxDistance / 2;
		for (const auto& Elem : RoomDepths)
		{
			if (Elem.Key == StartRoom)
			{
				continue;
			}

			if (Elem.Value == MidDist && !UsedRooms.Contains(Elem.Key))
			{
				Waypoints.Add(Elem.Key->GetEnemySpawnLocation());
			}
		}
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GS->SetExplorationWaypoints(Waypoints);
		UE_LOG(LogTemp, Log, TEXT("LG::ComputeExplorationWaypoints: Generated %d waypoints"), Waypoints.Num());

#if WITH_EDITOR
		GS->DrawDebugWaypoints();
#endif
	}
}
