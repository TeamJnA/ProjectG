// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitIronDoor.h"
#include "AbilitySystemComponent.h"

APGExitIronDoor::APGExitIronDoor()
{
    PrimaryActorTick.bCanEverTick = true;
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

    bDoorAutoClose = false;
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
            HandWheelLubricantPoint->SetRenderCustomDepth(true);
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
    IronChainMesh->SetRenderCustomDepth(false);
    HandWheelHole->SetRenderCustomDepth(false);
    HandWheelLubricantPoint->SetRenderCustomDepth(false);
}

FInteractionInfo APGExitIronDoor::GetInteractionInfo() const
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            UE_LOG(LogTemp, Log, TEXT("Unlock chain"));

            break;
        }
        case E_LockPhase::E_WheelAttach:
        {
            UE_LOG(LogTemp, Log, TEXT("Attach Wheel"));

            break;
        }
        case E_LockPhase::E_OilApplied:
        {
            UE_LOG(LogTemp, Log, TEXT("Oiled Wheel"));

            break;
        }
        case E_LockPhase::E_Unlocked:
        {
            // MaxDoorHeight
            // CurrentDoorHeight
            // 5.0f * ( MaxDoorHeight - CurrentDoorHeight ) / MaxDoorHeight
            const float UnlockDuration = 5.0f;
            const float UnlockStartTime = 5.0f * CurrentDoorHeight / MaxDoorHeight;

            UE_LOG(LogTemp, Log, TEXT("ExitIronDoor Unlock Duration : [%f], Unlock Start Time : [%f]"), UnlockDuration, UnlockStartTime);

            return FInteractionInfo(EInteractionType::Hold, UnlockDuration, UnlockStartTime);
        }
    }
    const float Duration = 3.0f;

    return FInteractionInfo(EInteractionType::Hold, Duration);
}

bool APGExitIronDoor::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
    switch (CurrentLockPhase)
    {
    case E_LockPhase::E_ChainLock:
    {
        if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.ChainKey"))))
        {
            UE_LOG(LogTemp, Log, TEXT("CanStartInteraction ChainLock"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Chain is locked"));

        return false;
    }
    case E_LockPhase::E_WheelAttach:
    {
        if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.Wheel"))))
        {
            UE_LOG(LogTemp, Log, TEXT("CanStartInteraction WheelPoint"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Required Wheel"));

        return false;
    }
    case E_LockPhase::E_OilApplied:
    {
        if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.RustOil"))))
        {
            UE_LOG(LogTemp, Log, TEXT("CanStartInteraction HandWheel"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Required Oil to remove rust"));

        return false;
    }
    case E_LockPhase::E_Unlocked:
    {
        UE_LOG(LogTemp, Log, TEXT("E_Unlocked"));
        break;
    }
    }
	return true;
}

void APGExitIronDoor::UpdateHoldProgress(float Progress)
{
    if (CurrentLockPhase == E_LockPhase::E_Unlocked)
    {
        // Rotate wheel
        
        // 1. 현재 컴포넌트의 회전을 쿼터니언으로 가져옵니다.
        FQuat CurrentQuat = HandWheelLubricantPoint->GetRelativeRotation().Quaternion();

        // 2. Pitch 축(일반적으로 Y축)을 기준으로 2.0도 회전하는 쿼터니언을 만듭니다.
        // FQuat 생성자는 각도를 라디안으로 받습니다.
        FQuat AdditiveRotation = FQuat(FVector::YAxisVector, FMath::DegreesToRadians(1.0f));

        // 3. 누적된 회전을 계산합니다. (회전 순서: 새로운 회전 * 현재 회전)
        FQuat NewQuat = AdditiveRotation * CurrentQuat;

        // 4. 새로운 쿼터니언 회전을 Rotator로 변환하여 설정합니다.
        HandWheelLubricantPoint->SetRelativeRotation(NewQuat.Rotator());

        // Door open
        if (bDoorAutoClose)
        {
            UE_LOG(LogTemp, Log, TEXT("Set bDoorAutoClose False"));
            bDoorAutoClose = false;
            SetActorTickEnabled(false);
        }
        CurrentDoorHeight = Progress * MaxDoorHeight;
        const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
        IronDoorMesh->SetRelativeLocation(NewDoorLocation);
    }
}

void APGExitIronDoor::StopHoldProress()
{
    UE_LOG(LogTemp, Log, TEXT("Exit Iron Door stop hold progress"));
    if (CurrentDoorHeight > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Set bDoorAutoClose True"));
        bDoorAutoClose = true;
        SetActorTickEnabled(true);
    }
}

bool APGExitIronDoor::Unlock()
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            CurrentLockPhase = E_LockPhase::E_WheelAttach;

            IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

            HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

            SetChainsUnlock();

            UE_LOG(LogTemp, Log, TEXT("Unlock chain"));

            return true;
        }
        case E_LockPhase::E_WheelAttach:
        {
            CurrentLockPhase = E_LockPhase::E_OilApplied;

            HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

            HandWheelLubricantPoint->SetVisibility(true);
            HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

            UE_LOG(LogTemp, Log, TEXT("Attach Wheel"));

            return true;
        }
        case E_LockPhase::E_OilApplied:
        {
            CurrentLockPhase = E_LockPhase::E_Unlocked;
            SetWheelMaterialOiled();

            UE_LOG(LogTemp, Log, TEXT("Oiled Wheel"));

            return true;
        }
        case E_LockPhase::E_Unlocked:
        {
            ///
            /// TODO : Open Door Completed by rotating wheel
            ///
            StopHoldProress();
            break;
        }
    }

    return false;
}

void APGExitIronDoor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    InitializeChainComponents();

    // set base start door location
    DoorBaseLocation = IronDoorMesh->GetRelativeLocation();
}

void APGExitIronDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UE_LOG(LogTemp, Log, TEXT("APGExitIronDoor bDoorAutoClose Tick Check"));

    // Close Door Automatically
    if (bDoorAutoClose)
    {
        CurrentDoorHeight -= DeltaSeconds * 6;

        const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
        IronDoorMesh->SetRelativeLocation(NewDoorLocation);

        if (CurrentDoorHeight <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("The iron door closed automatically"));

            CurrentDoorHeight = 0;
            SetActorTickEnabled(false);
            bDoorAutoClose = false;
        }
    }
}

void APGExitIronDoor::InitializeChainComponents()
{
    // Init chain meshes
    GetComponents<UStaticMeshComponent>(ChainMeshes);

    for (int32 i = ChainMeshes.Num() - 1; i >= 0; --i)
    {
        if (!ChainMeshes[i]->GetName().Contains(TEXT("Chain")))
        {
            ChainMeshes.RemoveAt(i);
        }
    }
}

void APGExitIronDoor::SetChainsUnlock()
{
    for (UStaticMeshComponent* ChainMesh : ChainMeshes)
    {
        if (ChainMesh)
        {
            ChainMesh->SetSimulatePhysics(true);
            ChainMesh->SetCollisionProfileName(TEXT("Item"));
            ChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        }
    }
}

void APGExitIronDoor::SetWheelMaterialOiled()
{
    if (HandWheelOiledMaterial)
    {
        HandWheelLubricantPoint->SetMaterial(0, HandWheelOiledMaterial);
    }
}
