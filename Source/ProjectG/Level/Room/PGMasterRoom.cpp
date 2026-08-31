// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGMasterRoom.h"
#include "Level/Misc/PGWall.h"
#include "Components/BoxComponent.h"

// Sets default values
APGMasterRoom::APGMasterRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(2.0f);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	GeometryFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GeometryFolder"));
	GeometryFolder->SetupAttachment(RootComponent);

	RoomDir = CreateDefaultSubobject<UArrowComponent>(TEXT("RoomDirection"));
	RoomDir->SetupAttachment(RootComponent);

	OverlapBoxFolder = CreateDefaultSubobject<USceneComponent>(TEXT("OverlapBoxFolder"));
	OverlapBoxFolder->SetupAttachment(RootComponent);

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(OverlapBoxFolder);

	ExitPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("ExitsFolder"));
	ExitPointsFolder->SetupAttachment(RootComponent);

	SearchableSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("SearchableSpawnPointsFolder"));
	SearchableSpawnPointsFolder->SetupAttachment(RootComponent);

	GimmickSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GimmickSpawnPointsFolder"));
	GimmickSpawnPointsFolder->SetupAttachment(RootComponent);

	FuseBoxSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("FuseBoxSpawnPointsFolder"));
	FuseBoxSpawnPointsFolder->SetupAttachment(RootComponent);

	PropsSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("PropsSpawnPointsFolder"));
	PropsSpawnPointsFolder->SetupAttachment(RootComponent);

	GlassBottleSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GlassBottleSpawnPointsFolder"));
	GlassBottleSpawnPointsFolder->SetupAttachment(RootComponent);

	BloodstainSpawnPointsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("BloodstainSpawnPointsFolder"));
	BloodstainSpawnPointsFolder->SetupAttachment(RootComponent);

	RoomDir->SetRelativeLocation(FVector(720.0f, 0.0f, 130.0f));
	RoomDir->SetRelativeScale3D(FVector(7.75f, 7.75f, 7.75f));

	OverlapBox->CanCharacterStepUpOn = ECB_No;
	OverlapBox->SetCollisionObjectType(ECC_GameTraceChannel1);
	OverlapBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	OverlapBox->SetRelativeLocation(FVector(990.0f, 0.0f, 0.0f));
	OverlapBox->SetRelativeScale3D(FVector(30.0f, 30.0f, 1.0f));

	EnemySpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("EnemySpawnPoint"));
	EnemySpawnPoint->SetupAttachment(RootComponent);
	EnemySpawnPoint->SetArrowColor(FLinearColor(0.2f, 0.0f, 1.0f, 0.0f));
	EnemySpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	EnemySpawnPoint->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	EnemySpawnPoint->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
	EnemySpawnPoint->bHiddenInGame = true;

	WallClass = APGWall::StaticClass();
}

void APGMasterRoom::GetOverlapBoxesForClass(TSubclassOf<APGMasterRoom> RoomClass, const FTransform& InRoomTransform, TArray<FPGRoomOverlapBox>& OutBoxes)
{
	OutBoxes.Reset();

	if (!RoomClass)
	{
		return;
	}

	const APGMasterRoom* CDO = RoomClass->GetDefaultObject<APGMasterRoom>();
	if (!CDO || !CDO->OverlapBoxFolder)
	{
		return;
	}

	// CDO는 컴포넌트가 등록되지 않아 AttachChildren이 비어있음 (Parent는 Children을 모르고 Children만 Parent를 아는 상태(SetupAttatchment))
	// 소유 컴포넌트를 전부 체크 후 AttachParent 체인을 거슬러 OverlapBoxFolder 하위인지 확인
	TArray<UBoxComponent*> BoxComps;
	CDO->GetComponents<UBoxComponent>(BoxComps);

	for (const UBoxComponent* BoxComp : BoxComps)
	{
		FTransform LocalTransform = FTransform::Identity;
		bool bUnderOverlapFolder = false;

		for (const USceneComponent* Current = BoxComp; Current && Current != CDO->Root; Current = Current->GetAttachParent())
		{
			if (Current == CDO->OverlapBoxFolder)
			{
				bUnderOverlapFolder = true;
			}

			LocalTransform = LocalTransform * Current->GetRelativeTransform();
		}

		if (!bUnderOverlapFolder)
		{
			continue;
		}

		const FTransform WorldTransform = LocalTransform * InRoomTransform;

		FPGRoomOverlapBox& Out = OutBoxes.AddDefaulted_GetRef();
		Out.Location = WorldTransform.GetLocation();
		Out.Rotation = WorldTransform.GetRotation();
		Out.HalfExtent = BoxComp->GetUnscaledBoxExtent() * WorldTransform.GetScale3D();
	}
}
