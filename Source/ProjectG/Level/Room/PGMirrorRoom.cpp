// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Room/PGMirrorRoom.h"
#include "Components/BoxComponent.h"
#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickLever.h"
#include "Level/Misc/PGDoor1.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Components/ChildActorComponent.h"
#include "Kismet/KismetMathLibrary.h"

APGMirrorRoom::APGMirrorRoom()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/ProjectG/Levels/LevelInstance/LI_MansionMirrorRoom.LI_MansionMirrorRoom_C'"));

	PrimaryActorTick.bCanEverTick = false;

	RoomDir->SetRelativeLocation(FVector(370.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(641.5f, 600.0f, 345.0f));
	OverlapBox->SetRelativeScale3D(FVector(19.75f, 29.25f, 10.5f));

	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(643.5f, 560.0f, -16.7f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	EntryTrigger->SetupAttachment(Root);
	EntryTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(Root);
	GateMesh->SetCollisionProfileName(TEXT("BlockAll"));

	GhostSpawnPointFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GhostSpawnPoints"));
	GhostSpawnPointFolder->SetupAttachment(Root);

	LeverComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Lever"));
	LeverComponent->SetupAttachment(Root);
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

	TArray<USceneComponent*> SpawnPoints = GhostSpawnPointFolder->GetAttachChildren();
	if (SpawnPoints.Num() == 0)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[MirrorRoom] StartGimmick"));

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

	APGMirrorGhostCharacter* NewGhost = GetWorld()->SpawnActor<APGMirrorGhostCharacter>(GhostClass, SpawnTransform, SpawnParams);
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
}