// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitIronDoor.h"

APGExitIronDoor::APGExitIronDoor()
{
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMesh->SetupAttachment(Root);
	PillarMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	IronChainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChainMesh"));
	IronChainMesh->SetupAttachment(PillarMesh);
	IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HandWheelHole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelHole"));
	HandWheelHole->SetupAttachment(PillarMesh);
	HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	HandWheelLubricantPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelLubricantPoint"));
	HandWheelLubricantPoint->SetupAttachment(PillarMesh);
	HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	IronDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronDoorMesh"));
	IronDoorMesh->SetupAttachment(Root);
	IronDoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	CurrentLockPhase = E_LockPhase::E_ChainLock;
}

void APGExitIronDoor::HighlightOn() const
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            IronChainMesh->SetRenderCustomDepth(true);
            break;
        }
        case E_LockPhase::E_WheelAttach:
        {
            HandWheelHole->SetRenderCustomDepth(true);
            break;
        }
        case E_LockPhase::E_OilApplied:
        {
            HandWheelLubricantPoint->SetRenderCustomDepth(true);
            break;
        }
        case E_LockPhase::E_Unlocked:
        {
            break;
        }
        default:
        {
            UE_LOG(LogTemp, Error, TEXT(""));
            break;
        }
    }
}

void APGExitIronDoor::HighlightOff() const
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            IronChainMesh->SetRenderCustomDepth(false);
            break;
        }
        case E_LockPhase::E_WheelAttach:
        {
            HandWheelHole->SetRenderCustomDepth(false);
            break;
        }
        case E_LockPhase::E_OilApplied:
        {
            HandWheelLubricantPoint->SetRenderCustomDepth(false);
            break;
        }
        case E_LockPhase::E_Unlocked:
        {
            break;
        }
        default:
        {
            UE_LOG(LogTemp, Error, TEXT(""));
            break;
        }
    }
}

FInteractionInfo APGExitIronDoor::GetInteractionInfo() const
{
    const float Duration = 3.0f;

    return FInteractionInfo(EInteractionType::Hold, Duration);
}

bool APGExitIronDoor::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	return true;
}

void APGExitIronDoor::Unlock()
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            CurrentLockPhase = E_LockPhase::E_WheelAttach;

            IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            IronChainMesh->SetVisibility(false);

            HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

            UE_LOG(LogTemp, Log, TEXT("Unlock chain"));
            /*
            TODO : unlock chain
            */
            break;
        }
        case E_LockPhase::E_WheelAttach:
        {
            CurrentLockPhase = E_LockPhase::E_OilApplied;

            HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

            HandWheelLubricantPoint->SetVisibility(true);
            HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

            UE_LOG(LogTemp, Log, TEXT("Attach Wheel"));

            break;
        }
        case E_LockPhase::E_OilApplied:
        {
            CurrentLockPhase = E_LockPhase::E_Unlocked;

            break;
        }
        case E_LockPhase::E_Unlocked:
        {
            ///
            /// TODO : Open Door by rotating wheel
            ///
            break;
        }
    }
}
