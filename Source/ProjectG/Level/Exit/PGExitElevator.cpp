// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitElevator.h"
#include "AbilitySystemComponent.h"
#include "PGLogChannels.h"

APGExitElevator::APGExitElevator()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

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

	FuseState = 0;
}

void APGExitElevator::HighlightOn() const
{
	FusePanel->SetRenderCustomDepth(true);
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
	if (FuseStatusAnim.IsValidIndex(FuseState) && FuseStatusAnim[FuseState])
	{
		FusePanel->SetAnimation(FuseStatusAnim[FuseState]);
		FusePanel->Play(false);

		FuseState++;

		if (FuseState == 1 || FuseState == 2)
		{
			return true;
		}
	}

	return false;
}
