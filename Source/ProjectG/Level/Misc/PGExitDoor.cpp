// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/PGExitDoor.h"

#include "Components/BoxComponent.h"

#include "AbilitySystemComponent.h"
#include "Interact/Ability/GA_Interact_ExitDoor.h"

#include "Character/PGPlayerCharacter.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"

#include "Net/UnrealNetwork.h"

// Sets default values
APGExitDoor::APGExitDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/Imports/SICKA_mansion/StaticMeshes/SM_DoorCarved.SM_DoorCarved'"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh0"));	
	Mesh0->SetupAttachment(Root);
	Mesh0->SetRelativeLocation(FVector(-163.0f, 22.1f, 0.0f));
	Mesh0->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.96f));

	Mesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh1"));
	Mesh1->SetupAttachment(Root);
	Mesh1->SetRelativeLocation(FVector(163.0f, 0.0f, 0.0f));
	Mesh1->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Mesh1->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.96f));

	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
		Mesh1->SetStaticMesh(MeshRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> EscapeMeshRef(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	static ConstructorHelpers::FObjectFinder<UMaterial> EscapeMeshMaterialRef(TEXT("/Script/Engine.Material'/Engine/EngineDebugMaterials/BlackUnlitMaterial.BlackUnlitMaterial'"));

	EscapeMesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EscapeMesh0"));
	EscapeMesh0->SetupAttachment(Root);
	EscapeMesh0->SetRelativeLocation(FVector(0.0f, -200.0f, 150.0f));
	EscapeMesh0->SetRelativeScale3D(FVector(3.5f, 4.0f, 3.25f));
	EscapeMesh0->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EscapeMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EscapeMesh1"));
	EscapeMesh1->SetupAttachment(Root);
	EscapeMesh1->SetRelativeLocation(FVector(0.0f, -200.0f, -10.0f));
	EscapeMesh1->SetRelativeScale3D(FVector(3.5f, 4.0f, 0.25f));
	EscapeMesh1->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	if (EscapeMeshRef.Succeeded())
	{
		EscapeMesh0->SetStaticMesh(EscapeMeshRef.Object);
		EscapeMesh1->SetStaticMesh(EscapeMeshRef.Object);
	}
	if (EscapeMeshMaterialRef.Succeeded())
	{
		EscapeMesh0->SetMaterial(0, EscapeMeshMaterialRef.Object);
		EscapeMesh1->SetMaterial(0, EscapeMeshMaterialRef.Object);
	}

	EscapeTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTriggerVolume"));
	EscapeTriggerVolume->SetupAttachment(Root);
	EscapeTriggerVolume->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	EscapeTriggerVolume->SetBoxExtent(FVector(130.0f, 10.0f, 150.0f));
	EscapeTriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	EscapeTriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	InteractAbility = UGA_Interact_ExitDoor::StaticClass();

	LockStack = 3;
}

TSubclassOf<UGameplayAbility> APGExitDoor::GetAbilityToInteract() const
{
	return InteractAbility;
}

/*
* 
*/
void APGExitDoor::HighlightOn() const
{
	Mesh0->SetRenderCustomDepth(true);
	Mesh1->SetRenderCustomDepth(true);
}

/*
* 
*/
void APGExitDoor::HighlightOff() const
{
	Mesh0->SetRenderCustomDepth(false);
	Mesh1->SetRenderCustomDepth(false);
}

/*
* Exit Door의 InteractionInfo 반환
* Hold Input
* 잠김 -> 3초 홀드
* 잠김 x -> 1초 홀드
*/
FInteractionInfo APGExitDoor::GetInteractionInfo() const
{
	const float Duration = (LockStack > 0) ? 3.0f : 1.0f;
	return FInteractionInfo(EInteractionType::Hold, Duration);
}

/*
* 잠김
*	상호작용 시도 플레이어가 ExitKey를 들고 있으면 상호작용 가능
*	상호작용 시도 플레이어가 ExitKey를 들고 있지	않으면 상호작용 불가능, 실패 메시지 return
* 잠김 x
*	상호작용 가능
* 
* 이미 열려있는 경우
*	상호작용 불가능
*/
bool APGExitDoor::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{	
	if (bIsOpen)
	{
		OutFailureMessage = FText::GetEmpty();
		return false;
	}
	
	// if exit door is locked
	if (LockStack > 0)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ExitKey"))))
		{
			return true;
		}
		OutFailureMessage = FText::FromString(TEXT("Door is locked"));
		return false;
	}

	return true;
}

bool APGExitDoor::IsLocked() const
{
	UE_LOG(LogTemp, Log, TEXT("ExitDoor::IsLocked: [SERVER] Current LockStack = %d"), LockStack);

	return LockStack > 0;
}

bool APGExitDoor::IsOpened() const
{
	UE_LOG(LogTemp, Log, TEXT("ExitDoor::IsOpened: door %s"), bIsOpen ? TEXT("Opened") : TEXT("Closed"));

	return bIsOpen;
}

void APGExitDoor::ToggleDoor()
{
	bIsOpen = true;

	UE_LOG(LogTemp, Log, TEXT("ExitDoor::ToggleDoor: [SERVER] Open Door"));

	Mesh0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void APGExitDoor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	EscapeTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APGExitDoor::OnEscapeTriggerOverlap);
}

void APGExitDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGExitDoor, bIsOpen);
	DOREPLIFETIME(APGExitDoor, LockStack);
}

void APGExitDoor::OnRep_LockStack()
{
	UE_LOG(LogTemp, Log, TEXT("ExitDoor::OnRep_LockStack: [CLIENT] LockStack = %d"), LockStack);
}

void APGExitDoor::OnRep_DoorState()
{
	UE_LOG(LogTemp, Log, TEXT("ExitDoor::OnRep_DoorState: [Client] Open Door"));

	Mesh0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh1->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

/*
* 플레이어가 ExitDoor를 열고 트리거에 닿은 경우 종료처리
*/
void APGExitDoor::OnEscapeTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsOpen)
	{
		return;
	}
	// 이하 삭제됨.
}
