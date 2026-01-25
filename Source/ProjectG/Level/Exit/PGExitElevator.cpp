// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitElevator.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"
#include "Character/PGPlayerCharacter.h"

APGExitElevator::APGExitElevator()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	bReplicates = true;

	ElevatorBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorBody"));
	ElevatorBody->SetupAttachment(Root);
	ElevatorBody->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	InnerFenceBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFenceBase"));
	InnerFenceBase->SetupAttachment(ElevatorBody);
	InnerFenceBase->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	InnerFenceDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InnerFenceDoor"));
	InnerFenceDoor->SetupAttachment(ElevatorBody);
	InnerFenceDoor->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	FusePanel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FusePanel"));
	FusePanel->SetupAttachment(ElevatorBody);
	FusePanel->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	OuterFenceBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterFenceBase"));
	OuterFenceBase->SetupAttachment(Root);
	OuterFenceBase->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	OuterFenceDoor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OuterFenceDoor"));
	OuterFenceDoor->SetupAttachment(Root);
	OuterFenceDoor->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	FusePanelCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("FusePanelCollision"));
	FusePanelCollision->SetupAttachment(FusePanel);

	ElevatorBodyCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ElevatorBodyCollision"));
	ElevatorBodyCollision->SetupAttachment(ElevatorBody);
	

	FuseState = 0;

	bInnerDoorClosed = false;

	ExitPointType = EExitPointType::Elevator;
}

void APGExitElevator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGExitElevator, FuseState);
}

void APGExitElevator::BeginPlay()
{
	Super::BeginPlay();

	BaseInnerFenceLocation = InnerFenceDoor->GetRelativeLocation();
	BaseOuterFenceLocation = OuterFenceDoor->GetRelativeLocation();

	// Set timelines
	if (DoorCloseCurveFloat)
	{
		FOnTimelineFloat ProgressUpdate;
		ProgressUpdate.BindUFunction(this, FName("DoorCloseProgress"));

		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindUFunction(this, FName("DoorCloseFinished"));

		DoorCloseTimeline.AddInterpFloat(DoorCloseCurveFloat, ProgressUpdate);
		DoorCloseTimeline.SetTimelineFinishedFunc(FinishedEvent);

		DoorCloseTimeline.SetLooping(false);
	}

	if (ElevatorDescentCurveFloat)
	{
		FOnTimelineFloat ProgressUpdate;
		ProgressUpdate.BindUFunction(this, FName("ElevatorDescentProgress"));

		FOnTimelineEvent FinishedEvent;
		FinishedEvent.BindUFunction(this, FName("ElevatorDescentFinished"));

		ElevatorDescentTimeline.AddInterpFloat(ElevatorDescentCurveFloat, ProgressUpdate);
		ElevatorDescentTimeline.SetTimelineFinishedFunc(FinishedEvent);

		ElevatorDescentTimeline.SetLooping(false);
	}
}

void APGExitElevator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (DoorCloseTimeline.IsPlaying())
	{
		DoorCloseTimeline.TickTimeline(DeltaTime);
	}

	if (ElevatorDescentTimeline.IsPlaying())
	{
		ElevatorDescentTimeline.TickTimeline(DeltaTime);
	}
}

void APGExitElevator::HighlightOn() const
{
	if(FuseState <= 2)
	{
		FusePanel->SetRenderCustomDepth(true);
	}
}

void APGExitElevator::HighlightOff() const
{
	FusePanel->SetRenderCustomDepth(false);
}

FInteractionInfo APGExitElevator::GetInteractionInfo() const
{
	const float Duration = 1.5;

	return FInteractionInfo(EInteractionType::Hold, Duration);
}

bool APGExitElevator::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	// Fuse를 껴야하는 단계
	if (FuseState <= 1)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.Fuse"))))
		{
			UE_LOG(LogPGExitPoint, Log, TEXT("PGExitElevator CanStartInteraction Fuse"));
			return true;
		}
		OutFailureMessage = FText::FromString(TEXT("Require Fuse"));
	}
	else if (FuseState > 1)
	{
		UE_LOG(LogPGExitPoint, Log, TEXT("PGExitElevator CanStartInteraction Lever"));
		return true;
	}

	return false;
}

void APGExitElevator::InteractionFailed()
{
	// TODO : Play Tick(약간 전기 칙) 하는 소리 정도 ??
}

// true : remove item from inventory / false : do not remove item
bool APGExitElevator::Unlock()
{
	UE_LOG(LogPGExitPoint, Log, TEXT("APGExitElevator::Unlock. Current FuseState : [%d]"), FuseState);

	FuseState++;
	OnRep_FuseState();

	if (FuseState == 1 || FuseState == 2)
	{
		return true;
	}

	return false;
}

void APGExitElevator::OnRep_FuseState()
{
	const int32 FuseIndex = FuseState - 1;

	if (FuseStatusAnim.IsValidIndex(FuseIndex) && FuseStatusAnim[FuseIndex])
	{
		FusePanel->SetAnimation(FuseStatusAnim[FuseIndex]);
		FusePanel->Play(false);

		if (FuseIndex == 2)
		{
			FusePanelCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
			HighlightOff();

			ExecuteEscapeSequence();
		}
	}
}

void APGExitElevator::ExecuteEscapeSequence()
{
	SetActorTickEnabled(true);
	DoorCloseTimeline.PlayFromStart();
	ElevatorDescentTimeline.PlayFromStart();

	GetWorldTimerManager().SetTimer(EscapeTimerHandle, this, &APGExitElevator::EscapePlayers, 6.0f, false);
}

void APGExitElevator::EscapePlayers()
{
	if (!ElevatorBodyCollision) return;

	// 콜리전 내의 APGPlayerCharacter들을 담을 배열
	TArray<AActor*> OverlappingActors;

	// APGPlayerCharacter 클래스를 상속받은 액터들만 필터링하여 수집
	ElevatorBodyCollision->GetOverlappingActors(OverlappingActors, APGPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		APGPlayerCharacter* TargetPlayer = Cast<APGPlayerCharacter>(Actor);
		if (TargetPlayer)
		{
			OnEscapeStart(TargetPlayer, ExitPointType);
		}
	}
}

void APGExitElevator::DoorCloseProgress(float Value)
{
	if(!bInnerDoorClosed)
	{
		float NewDoorX = (InnerFenceClosedX - BaseInnerFenceLocation.X) * Value;
		FVector NewDoorLocation = BaseInnerFenceLocation;
		NewDoorLocation.X = BaseInnerFenceLocation.X + NewDoorX;

		InnerFenceDoor->SetRelativeLocation(NewDoorLocation);
	}
	else
	{
		float NewDoorX = (OuterFenceClosedX - BaseOuterFenceLocation.X) * Value;
		FVector NewDoorLocation = BaseOuterFenceLocation;
		NewDoorLocation.X = BaseOuterFenceLocation.X + NewDoorX;

		OuterFenceDoor->SetRelativeLocation(NewDoorLocation);
	}
}

void APGExitElevator::DoorCloseFinished()
{
	UE_LOG(LogPGExitPoint, Log, TEXT("APGExitElevator::DoorCloseFinished. InnerDoorClosed : [%d]"), bInnerDoorClosed);
	if (!bInnerDoorClosed)
	{
		bInnerDoorClosed = true;
		DoorCloseTimeline.PlayFromStart();
	}
}

void APGExitElevator::ElevatorDescentProgress(float Value)
{
	float NewElevatorZ = ElevatorDescentTargetZ * Value * (-1);
	ElevatorBody->SetRelativeLocation(FVector( 0.0f, 0.0f, NewElevatorZ));
}

void APGExitElevator::ElevatorDescentFinished()
{
	SetActorTickEnabled(false);
}
