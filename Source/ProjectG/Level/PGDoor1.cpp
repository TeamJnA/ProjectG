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

	DoorHinge = CreateDefaultSubobject<USceneComponent>(TEXT("DoorHinge"));
	DoorHinge->SetupAttachment(Root);
	DoorHinge->SetRelativeLocation(FVector(0.0f, 82.0f, 0.0f));

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	Mesh0->SetupAttachment(DoorHinge);
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	Mesh0->SetRelativeLocation(FVector(11.0f, 0.0f, 7.0f));	
	Mesh0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.985f));

	Mesh0->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);

	// Door does not affect to NavMesh. AI ignore door.
	Mesh0->SetCanEverAffectNavigation(false);

	const FRotator ClosedRotation = FRotator::ZeroRotator;
	const FVector ClosedLocation = FVector(0.0f, 82.0f, 0.0f);
	ClosedTransform = FTransform(ClosedRotation, ClosedLocation);

	const FRotator OpenedRotation_A = FRotator(0.0f, 90.0f, 0.0f);
	const FRotator OpenedRotation_B = FRotator(0.0f, -90.0f, 0.0f);
	const FVector OpenedLocation = FVector(0.0f, 74.0f, 0.0f);
	OpenedTransform_A = FTransform(OpenedRotation_A, OpenedLocation);
	OpenedTransform_B = FTransform(OpenedRotation_B, OpenedLocation);

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

void APGDoor1::ToggleDoor(AActor* InteractInvestigator)
{
	SetDoorState(!bIsOpen, InteractInvestigator);
}

void APGDoor1::SetDoorState(bool InbIsOpen, AActor* InteractInvestigator)
{
	bIsOpen = InbIsOpen;
	if (bIsOpen)
	{
		Mesh0->SetCanEverAffectNavigation(true);

		if (InteractInvestigator)
		{
			const FVector DoorToCharacter = InteractInvestigator->GetActorLocation() - GetActorLocation();
			const FVector DoorForwardVector = GetActorForwardVector();
			const float DotProduct = FVector::DotProduct(DoorForwardVector, DoorToCharacter);

			DesiredTransform = (DotProduct < 0.0f) ? OpenedTransform_A : OpenedTransform_B;
		}
		else
		{
			DesiredTransform = OpenedTransform_A;
		}
	}
	else
	{
		Mesh0->SetCanEverAffectNavigation(false);

		DesiredTransform = ClosedTransform;
	}

	OnRep_DesiredTransform();
}

void APGDoor1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGDoor1, bIsOpen);
	DOREPLIFETIME(APGDoor1, bIsLocked);
	DOREPLIFETIME(APGDoor1, DesiredTransform);
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

void APGDoor1::OnRep_DesiredTransform()
{
	DoorHinge->SetRelativeTransform(DesiredTransform);
}

// Client action after change lock state
void APGDoor1::OnRep_LockState()
{
	UE_LOG(LogTemp, Log, TEXT("Door lock state changed: %s"), bIsLocked ? TEXT("Locked") : TEXT("Unlocked"));
}

void APGDoor1::TEST_OpenDoorByAI(AActor* InteractInvestigator)
{
	UE_LOG(LogTemp, Log, TEXT("OpenDoor by AI"));
	SetDoorState(true, InteractInvestigator);
}