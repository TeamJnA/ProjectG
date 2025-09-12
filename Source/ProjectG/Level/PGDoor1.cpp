// Fill out your copyright notice in the Description page of Project Settings.


#include "PGDoor1.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

#include "AbilitySystemComponent.h"
#include "Interact/Ability/GA_Interact_Door.h"

#include "Net/UnrealNetwork.h"

// Sets default values
APGDoor1::APGDoor1()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;


	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/Imports/SICKA_mansion/StaticMeshes/SM_DoorCarved.SM_DoorCarved'"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Root->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	Root->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	Root->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	Mesh0->SetupAttachment(Root);
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	Mesh0->SetRelativeLocation(FVector(11.0f, 81.8f, 7.0f));	
	Mesh0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.985f));

	Mesh0->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);

	// Door does not affect to NavMesh. AI ignore door.
	Mesh0->SetCanEverAffectNavigation(false);

	InteractAbility = UGA_Interact_Door::StaticClass();
}

void APGDoor1::SpawnDoor(UWorld* World, const FTransform& Transform, const FActorSpawnParameters& SpawnParams, bool InbIsLocked)
{
	APGDoor1* NewDoor = World->SpawnActor<APGDoor1>(StaticClass(), Transform, SpawnParams);
	if (NewDoor)
	{
		NewDoor->bIsLocked = InbIsLocked;
	}
}

TSubclassOf<UGameplayAbility> APGDoor1::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGDoor1::ToggleDoor()
{
	SetDoorState(!bIsOpen);
}

void APGDoor1::SetDoorState(bool InbIsOpen)
{
	bIsOpen = InbIsOpen;

	FRotator NewRot = InbIsOpen ? FRotator(0.0f, 180.0f, 0.0f) : FRotator(0.0f, 90.0f, 0.0f);
	Mesh0->SetRelativeRotation(NewRot);
}

void APGDoor1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGDoor1, bIsOpen);
	DOREPLIFETIME(APGDoor1, bIsLocked);
}

/*
*
*/
void APGDoor1::HighlightOn() const
{
	Mesh0->SetRenderCustomDepth(true);
}

/*
* 
*/
void APGDoor1::HighlightOff() const
{
	Mesh0->SetRenderCustomDepth(false);
}

/*
* 일반 Door의 InteractionInfo 반환
* HoldInput
* 잠김 -> 1초 홀드
* 잠김 x -> 0.3초 홀드
*/
FInteractionInfo APGDoor1::GetInteractionInfo() const
{
	const float Duration = bIsLocked ? 1.0f : 0.3f;
	return FInteractionInfo(EInteractionType::Hold, Duration);
}

/*
* 잠긴 문인 경우
*	상호작용 시도 플레이어가 Key를 들고 있으면 상호작용 가능
*	상호작용 시도 플레이어가 Key를 들고 있지 않으면 상호작용 불가능, 실패 메시지 return
* 잠기지 않은 문인 경우
*	상호작용 가능
*/
bool APGDoor1::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (bIsLocked)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Key"))))
		{
			return true;
		}
		OutFailureMessage = FText::FromString(TEXT("Door is locked"));

		return false;
	}

	return true;
}

// Client action after toggle door
void APGDoor1::OnRep_DoorState()
{
	//SetDoorState(bIsOpen);

	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("APGDoor1::OnRep_DoorState - Client received replication for %s. New bIsOpen: %s."),
			*GetName(), bIsOpen ? TEXT("OPEN") : TEXT("CLOSED"));

		FRotator NewRot = bIsOpen ? FRotator(0.0f, 180.0f, 0.0f) : FRotator(0.0f, 90.0f, 0.0f);
		Mesh0->SetRelativeRotation(NewRot);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APGDoor1::OnRep_DoorState - Server received OnRep_DoorState (SHOULD NOT HAPPEN)."), *GetName());
	}
}

// Client action after change lock state
void APGDoor1::OnRep_LockState()
{
	UE_LOG(LogTemp, Log, TEXT("Door lock state changed: %s"), bIsLocked ? TEXT("Locked") : TEXT("Unlocked"));
}

void APGDoor1::TEST_OpenDoorByAI()
{
	UE_LOG(LogTemp, Log, TEXT("OpenDoor by AI"));
	SetDoorState(true);
}