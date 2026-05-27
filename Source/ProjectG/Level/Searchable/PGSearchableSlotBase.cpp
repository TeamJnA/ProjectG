// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Searchable/PGSearchableSlotBase.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Components/ArrowComponent.h"
#include "PGLogChannels.h"

// Sets default values
APGSearchableSlotBase::APGSearchableSlotBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SlotMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh1"));
	SetRootComponent(SlotMesh1);

	SlotMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh2"));
	SlotMesh2->SetupAttachment(SlotMesh1);

	ItemSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSpawnPoint"));
	ItemSpawnPoint->SetupAttachment(SlotMesh1);

	OpenDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OpenDirectionArrow"));
	OpenDirectionArrow->SetupAttachment(SlotMesh1);

	MovementTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MovementTimeline"));

	bIsDrawn = false;

	DrawLength = 100.0f;
}

void APGSearchableSlotBase::InteractSlot()
{
	check(HasAuthority());

	if (!bIsDrawn)
	{
		bIsDrawn = true;
		OnRep_IsDrawn();
	}
}

void APGSearchableSlotBase::AttachSpawnedItem(AActor* Item)
{
	if (IsValid(Item) && IsValid(ItemSpawnPoint))
	{
		Item->AttachToComponent(ItemSpawnPoint, FAttachmentTransformRules::KeepWorldTransform);

		// 아이템이 서랍과 함께 부드럽게 움직이도록 물리/리플리케이트 설정
		Item->SetReplicateMovement(false);
	}
}

TSubclassOf<UGameplayAbility> APGSearchableSlotBase::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGSearchableSlotBase::HighlightOn() const
{
	if (SlotMesh1)
	{
		SlotMesh1->SetCustomDepthStencilValue(0);
		SlotMesh1->SetRenderCustomDepth(true);
	}
	if (SlotMesh2)
	{
		SlotMesh2->SetCustomDepthStencilValue(0);
		SlotMesh2->SetRenderCustomDepth(true);
	}
}

void APGSearchableSlotBase::HighlightOff() const
{
	if (SlotMesh1)
	{
		SlotMesh1->SetRenderCustomDepth(false);
	}
	if (SlotMesh2)
	{
		SlotMesh2->SetRenderCustomDepth(false);
	}
}

FInteractionInfo APGSearchableSlotBase::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Instant);
}

FText APGSearchableSlotBase::GetInteractionText() const
{
	// TODO : Open 이나 Draw로 변수에 맞춰.
	return FText::FromString(TEXT("Open"));
}

bool APGSearchableSlotBase::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
{
	if (!InteractAbility)
	{
		OutFailurePrompt.Icon = nullptr;
		OutFailurePrompt.IconSize = FVector2D::ZeroVector;
		return false;
	}
	return true;
}

// Called when the game starts or when spawned
void APGSearchableSlotBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 월드 기준 초기 위치 및 정면(Forward Vector) 방향으로 100만큼 앞선 목표 위치 계산
	InitialLocation = GetActorLocation();
	TargetLocation = InitialLocation + (OpenDirectionArrow->GetForwardVector() * DrawLength);

	if (DrawCurve)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindDynamic(this, &APGSearchableSlotBase::UpdateTimeline);
		MovementTimeline->AddInterpFloat(DrawCurve, TimelineProgress);

		// Timeline이 마지막 key 값에 끝나도록
		MovementTimeline->SetTimelineLengthMode(TL_LastKeyFrame);
	}
}

void APGSearchableSlotBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSearchableSlotBase, bIsDrawn);
}

void APGSearchableSlotBase::OnRep_IsDrawn()
{
	if (MovementTimeline && DrawCurve)
	{
		MovementTimeline->Play();
	}

	HighlightOff();

	if (SlotMesh1)
	{
		SlotMesh1->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		SlotMesh1->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		SlotMesh1->SetCollisionResponseToChannel(ECC_ThrownItem, ECR_Block);
	}
	if (SlotMesh2)
	{
		SlotMesh2->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		SlotMesh2->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		SlotMesh2->SetCollisionResponseToChannel(ECC_ThrownItem, ECR_Block);
	}
}

void APGSearchableSlotBase::UpdateTimeline(float Value)
{
	FVector NewLocation = FMath::Lerp(InitialLocation, TargetLocation, Value);

	SetActorLocation(NewLocation);
}

