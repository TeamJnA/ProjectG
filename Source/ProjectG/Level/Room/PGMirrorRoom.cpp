// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGMirrorRoom.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"
#include "NavAreas/NavArea_Default.h"

#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickLever.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Kismet/KismetMathLibrary.h"

#include "GameFramework/GameModeBase.h"
#include "Sound/PGSoundManager.h"
#include "Interface/SoundManagerInterface.h"

APGMirrorRoom::APGMirrorRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/Room/LevelInstance/LI_MansionMirrorRoom.LI_MansionMirrorRoom_C'"));
	static ConstructorHelpers::FClassFinder<AActor> LeverRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Gimmick/Interact/Lever/BP_PGInteractableGimmickLever.BP_PGInteractableGimmickLever_C'"));
	static ConstructorHelpers::FClassFinder<AActor> MirrorGhostRef(TEXT("/Game/ProjectG/Enemy/MirrorGhost/Character/BP_PGMirrorGhostCharacter.BP_PGMirrorGhostCharacter_C"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GateMeshRef(TEXT("/Game/Imports/ExitPoint/Fence/courtyard_hollowed_out_fence.courtyard_hollowed_out_fence"));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> GateMeshMaterialRef(TEXT("/Game/Imports/Materials/RustyMaterials/rusty_metal_sheet_tjylcjvn_1k/MI_RustyMetalSheet.MI_RustyMetalSheet"));

	PrimaryActorTick.bCanEverTick = false;

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(641.5f, 600.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.75f, 29.25f, 10.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	if (MeshRef.Succeeded())
	{
		Mesh->SetChildActorClass(MeshRef.Class);
	}
	Mesh->SetRelativeLocation(FVector(643.5f, 560.0f, 0.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	EntryTrigger->SetupAttachment(Root);
	EntryTrigger->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	EntryTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EntryTrigger->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EntryTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
	EntryTrigger->SetRelativeLocation(FVector(650.0f, 600.0f, 165.0f));
	EntryTrigger->SetRelativeScale3D(FVector(17.5f, 27.5f, 5.0f));

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(Root);
	if (GateMeshRef.Succeeded())
	{
		GateMesh->SetStaticMesh(GateMeshRef.Object);
	}
	if (GateMeshMaterialRef.Succeeded())
	{
		GateMesh->SetMaterial(0, GateMeshMaterialRef.Object);
	}
	GateMesh->SetCollisionProfileName(TEXT("BlockAll"));
	GateMesh->SetRelativeLocation(FVector(15.0f, 0.0f, 320.0f));
	GateMesh->SetRelativeScale3D(FVector(1.0f, 1.6f, 2.05f));

	MirrorGhostSpawnPointFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GhostSpawnPoints"));
	MirrorGhostSpawnPointFolder->SetupAttachment(Root);

	MirrorGhostSpawnPoint0 = CreateDefaultSubobject<UArrowComponent>(TEXT("GhostSpawnPoint0"));
	MirrorGhostSpawnPoint0->SetupAttachment(MirrorGhostSpawnPointFolder);
	MirrorGhostSpawnPoint0->SetArrowColor(FLinearColor(0.2f, 0.0f, 1.0f, 0.0f));
	MirrorGhostSpawnPoint0->SetRelativeLocation(FVector(1000.0f, 1175.0f, 150.0f));
	MirrorGhostSpawnPoint0->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MirrorGhostSpawnPoint0->bHiddenInGame = true;

	MirrorGhostSpawnPoint1 = CreateDefaultSubobject<UArrowComponent>(TEXT("GhostSpawnPoint1"));
	MirrorGhostSpawnPoint1->SetupAttachment(MirrorGhostSpawnPointFolder);
	MirrorGhostSpawnPoint1->SetArrowColor(FLinearColor(0.2f, 0.0f, 1.0f, 0.0f));
	MirrorGhostSpawnPoint1->SetRelativeLocation(FVector(845.0f, 1175.0f, 150.0f));
	MirrorGhostSpawnPoint1->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MirrorGhostSpawnPoint1->bHiddenInGame = true;

	MirrorGhostSpawnPoint2 = CreateDefaultSubobject<UArrowComponent>(TEXT("GhostSpawnPoint2"));
	MirrorGhostSpawnPoint2->SetupAttachment(MirrorGhostSpawnPointFolder);
	MirrorGhostSpawnPoint2->SetArrowColor(FLinearColor(0.2f, 0.0f, 1.0f, 0.0f));
	MirrorGhostSpawnPoint2->SetRelativeLocation(FVector(630.0f, 1175.0f, 150.0f));
	MirrorGhostSpawnPoint2->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MirrorGhostSpawnPoint2->bHiddenInGame = true;

	MirrorGhostSpawnPoint3 = CreateDefaultSubobject<UArrowComponent>(TEXT("GhostSpawnPoint3"));
	MirrorGhostSpawnPoint3->SetupAttachment(MirrorGhostSpawnPointFolder);
	MirrorGhostSpawnPoint3->SetArrowColor(FLinearColor(0.2f, 0.0f, 1.0f, 0.0f));
	MirrorGhostSpawnPoint3->SetRelativeLocation(FVector(440.0f, 1175.0f, 150.0f));
	MirrorGhostSpawnPoint3->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MirrorGhostSpawnPoint3->bHiddenInGame = true;

	LeverComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Lever"));
	LeverComponent->SetupAttachment(Root);
	if (LeverRef.Succeeded())
	{
		LeverComponent->SetChildActorClass(LeverRef.Class);
	}
	LeverComponent->SetRelativeLocation(FVector(1162.0f, 1313.0f, 216.0f));
	LeverComponent->SetRelativeRotation(FRotator(0.0f, -65.0f, -3.0f));
	LeverComponent->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	NavModifier->SetAreaClass(UNavArea_Null::StaticClass());

	if (MirrorGhostRef.Succeeded())
	{
		MirrorGhostClass = MirrorGhostRef.Class;
	}

	DoorCloseSound = FName(TEXT("LEVEL_MirrorRoom_DoorClose"));
}

void APGMirrorRoom::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] begin play"));

	if (HasAuthority())
	{
		EntryTrigger->OnComponentBeginOverlap.AddDynamic(this, &APGMirrorRoom::OnEntryTriggerOverlap);
		EntryTrigger->OnComponentEndOverlap.AddDynamic(this, &APGMirrorRoom::OnEntryTriggerEndOverlap);

		if (LeverComponent)
		{
			if (APGInteractableGimmickLever* Lever = Cast<APGInteractableGimmickLever>(LeverComponent->GetChildActor()))
			{
				Lever->SetMasterRoom(this);
			}
		}
	}
}

void APGMirrorRoom::OnEntryTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsLocked || bIsSolved) 
	{
		UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] cannot start overlap"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] overlap start"));

	APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	APGPlayerState* PS = Player->GetPlayerState<APGPlayerState>();
	if (!PS || PS->IsDead())
	{
		return;
	}

	if (!GetWorld()->GetTimerManager().IsTimerActive(LockTriggerTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(LockTriggerTimerHandle, this, &APGMirrorRoom::StartGimmick, 2.0f, false);
	}
}

void APGMirrorRoom::OnEntryTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIsLocked || bIsSolved)
	{
		UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] cannot end overlap"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] overlap end"));

	APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	EntryTrigger->GetOverlappingActors(OverlappingActors, APGPlayerCharacter::StaticClass());

	bool bAnyPlayerAlive = false;
	for (AActor* Actor : OverlappingActors)
	{
		APGPlayerCharacter* RemainingPlayer = Cast<APGPlayerCharacter>(Actor);
		if (!RemainingPlayer)
		{
			UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] not player"));

			continue;
		}

		APGPlayerState* PS = RemainingPlayer->GetPlayerState<APGPlayerState>();
		if (!PS || PS->IsDead())
		{
			continue;
		}

		bAnyPlayerAlive = true;
		break;
	}

	if (!bAnyPlayerAlive)
	{
		GetWorld()->GetTimerManager().ClearTimer(LockTriggerTimerHandle);
	}
}

// ´ÝÈ÷´Â ºÎºÐ
void APGMirrorRoom::StartGimmick()
{
	if (bIsLocked || bIsSolved) 
	{
		return;
	}
	
	TArray<AActor*> OverlappingActors;
	EntryTrigger->GetOverlappingActors(OverlappingActors, APGPlayerCharacter::StaticClass());
	TArray<APGPlayerCharacter*> ValidTargets;
	for (AActor* Actor: OverlappingActors)
	{
		APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(Actor);
		if (!Player)
		{
			continue;
		}

		APGPlayerState* PS = Player->GetPlayerState<APGPlayerState>();
		if (!PS || PS->IsDead())
		{
			continue;
		}

		ValidTargets.Add(Player);
	}

	if (ValidTargets.Num() == 0)
	{
		return;
	}

	TArray<USceneComponent*> SpawnPoints = MirrorGhostSpawnPointFolder->GetAttachChildren();
	if (SpawnPoints.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] StartGimmick"));

	//Play Gate Close Sound
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			SoundManager->PlaySoundForAllPlayers(DoorCloseSound, GateMesh->GetComponentLocation());
		}
	}

	bIsLocked = true;
	Multicast_SetGateState(true);
	
	for (APGPlayerCharacter* Target : ValidTargets)
	{
		int32 RandIdx = FMath::RandRange(0, SpawnPoints.Num() - 1);
		FTransform SpawnTransform = SpawnPoints[RandIdx]->GetComponentTransform();
		SpawnMirrorGhost(Target, SpawnTransform);
	}
}

void APGMirrorRoom::SpawnMirrorGhost(APGPlayerCharacter* Player, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APGMirrorGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGMirrorGhostCharacter>(MirrorGhostClass, SpawnTransform, SpawnParams);
	if (NewGhost)
	{
		NewGhost->SetTargetPlayer(Player);
		SpawnedGhosts.Add(NewGhost);
	}
}

void APGMirrorRoom::UpdateGateMovement()
{
	float DeltaTime = 0.02f;
	CurrentGateTime += DeltaTime;
	float Alpha = FMath::Clamp(CurrentGateTime / GateMoveDuration, 0.0f, 1.0f);

	FVector NewLoc = FMath::InterpEaseInOut(GateStartLoc, GateTargetLoc, Alpha, 2.0f);
	GateMesh->SetRelativeLocation(NewLoc);

	if (Alpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(GateMoveTimerHandle);
	}
}

void APGMirrorRoom::Multicast_SetGateState_Implementation(bool bLock)
{
	GateStartLoc = GateMesh->GetRelativeLocation();
	GateTargetLoc = bLock ? GateClosedRelativeLocation : GateOpenRelativeLocation;
	CurrentGateTime = 0.0f;

	GetWorld()->GetTimerManager().ClearTimer(GateMoveTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(GateMoveTimerHandle, this, &APGMirrorRoom::UpdateGateMovement, 0.02f, true);

	bIsLocked = bLock;
}

void APGMirrorRoom::SolveGimmick()
{
	if (bIsSolved)
	{
		return;
	}
	bIsSolved = true;
	bIsLocked = false;

	Multicast_SetGateState(false);

	for (APGMirrorGhostCharacter* Ghost : SpawnedGhosts)
	{
		if (Ghost && IsValid(Ghost))
		{
			Ghost->Destroy();
		}
	}
	SpawnedGhosts.Empty();

	NavModifier->SetAreaClass(UNavArea_Default::StaticClass());
}