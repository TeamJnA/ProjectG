// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/PGExitDoor.h"

#include "AbilitySystemComponent.h"
#include "Interact/Ability/GA_Interact_ExitDoor.h"

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

	InteractAbility = UGA_Interact_ExitDoor::StaticClass();

	LockStack = 3;
}

TSubclassOf<UGameplayAbility> APGExitDoor::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGExitDoor::HighlightOn() const
{
	Mesh0->SetRenderCustomDepth(true);
	Mesh1->SetRenderCustomDepth(true);
}

void APGExitDoor::HighlightOff() const
{
	Mesh0->SetRenderCustomDepth(false);
	Mesh1->SetRenderCustomDepth(false);
}

FInteractionInfo APGExitDoor::GetInteractionInfo() const
{
	const float Duration = IsLocked() ? 3.0f : 1.0f;
	return FInteractionInfo(EInteractionType::Hold, Duration);
}

bool APGExitDoor::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{	
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
