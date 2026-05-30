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

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SlotMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlotMesh1"));
	SlotMesh1->SetupAttachment(Root);

	ItemSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSpawnPoint"));
	ItemSpawnPoint->SetupAttachment(SlotMesh1);

	OpenDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("OpenDirectionArrow"));
	OpenDirectionArrow->SetupAttachment(Root);

	MovementTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MovementTimeline"));

	bIsDrawn = false;

	DrawLength = 100.0f;

	// Set base Slot Mesh Map
	const UEnum* SlotMeshEnum = StaticEnum<ESlotMeshType>();
	if (SlotMeshEnum)
	{
		for (int32 i = 0; i < SlotMeshEnum->NumEnums() - 1; ++i)
		{
			ESlotMeshType EnumValue = static_cast<ESlotMeshType>(SlotMeshEnum->GetValueByIndex(i));

			if (!SlotMeshMap.Contains(EnumValue))
			{
				SlotMeshMap.Add(EnumValue, FSlotVisualData());
			}
		}
	}
}

void APGSearchableSlotBase::SetSlotMeshTransform(const FTransform& NewTransform)
{
	SlotMeshTransform = NewTransform;

	if (HasAuthority())
	{
		OnRep_SlotMeshTransform();
	}
}

void APGSearchableSlotBase::OnRep_SlotMeshTransform()
{
	if (SlotMesh1)
	{
		SlotMesh1->SetRelativeTransform(SlotMeshTransform);
	}
}

void APGSearchableSlotBase::SetItemSpawnPointTransform(const FTransform& NewTransform)
{
	ItemSpawnTransform = NewTransform;

	if (HasAuthority())
	{
		OnRep_ItemSpawnTransform();
	}
}

void APGSearchableSlotBase::OnRep_ItemSpawnTransform()
{
	if (ItemSpawnPoint)
	{
		ItemSpawnPoint->SetRelativeTransform(ItemSpawnTransform);
	}
}

void APGSearchableSlotBase::SetCurrentSlotMesh(ESlotMeshType _InSlotMesh)
{
	check(HasAuthority());

	CurrentSlotMesh = _InSlotMesh;
	OnRep_SlotMesh();
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
	if (IsValid(Item) && IsValid(ItemSpawnPoint) && SlotInteractionType == ESlotInteractType::Draw)
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
}

void APGSearchableSlotBase::HighlightOff() const
{
	if (SlotMesh1)
	{
		SlotMesh1->SetRenderCustomDepth(false);
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
	
	MovementTimeline->SetTimelineLengthMode(TL_LastKeyFrame);
}

void APGSearchableSlotBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSearchableSlotBase, bIsDrawn);
	DOREPLIFETIME(APGSearchableSlotBase, SlotInteractionType);
	DOREPLIFETIME(APGSearchableSlotBase, ItemSpawnTransform);
	DOREPLIFETIME(APGSearchableSlotBase, SlotMeshTransform);
}

void APGSearchableSlotBase::OnRep_SlotMesh()
{
	if (!SlotMesh1)
	{
		return;
	}

	// TMap에 해당 테마가 세팅되어 있는지 확인
	if (const FSlotVisualData* VisualData = SlotMeshMap.Find(CurrentSlotMesh))
	{
		if (VisualData->Mesh)
		{
			SlotMesh1->SetStaticMesh(VisualData->Mesh);
		}

		if (VisualData->Material)
		{
			SlotMesh1->SetMaterial(0, VisualData->Material);
		}
	}
}

void APGSearchableSlotBase::OnRep_IsDrawn()
{
	// SlotInteractionType에 맞춰 타임라인 세팅
	SetupTimeline();

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
}

void APGSearchableSlotBase::SetupTimeline()
{
	if (SlotInteractionType == ESlotInteractType::Draw)
	{
		InitialLocation = GetActorLocation();
		TargetLocation = InitialLocation + (OpenDirectionArrow->GetForwardVector() * DrawLength);

		if (DrawCurve)
		{
			FOnTimelineFloat TimelineProgress;
			TimelineProgress.BindDynamic(this, &APGSearchableSlotBase::UpdateDrawTimeline);
			MovementTimeline->AddInterpFloat(DrawCurve, TimelineProgress);
		}
	}
	else if (SlotInteractionType == ESlotInteractType::Open)
	{
		if (SlotMesh1)
		{
			InitialRotation = SlotMesh1->GetRelativeRotation();
			TargetRotation = InitialRotation + FRotator(0.0f, OpenAngle, 0.0f);
		}

		if (OpenCurve)
		{
			FOnTimelineFloat TimelineProgress;
			TimelineProgress.BindDynamic(this, &APGSearchableSlotBase::UpdateOpenTimeline);
			MovementTimeline->AddInterpFloat(OpenCurve, TimelineProgress);
		}
	}
}

void APGSearchableSlotBase::UpdateDrawTimeline(float Value)
{
	FVector NewLocation = FMath::Lerp(InitialLocation, TargetLocation, Value);

	SetActorLocation(NewLocation);
}

void APGSearchableSlotBase::UpdateOpenTimeline(float Value)
{
	if (SlotMesh1)
	{
		FRotator NewRot1 = FMath::Lerp(InitialRotation, TargetRotation, Value);
		SlotMesh1->SetRelativeRotation(NewRot1);
	}
}