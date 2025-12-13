// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitIronDoor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "PGLogChannels.h"

APGExitIronDoor::APGExitIronDoor()
{
    PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMesh->SetupAttachment(Root);
	PillarMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    // Chain Lock
	IronChainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChainMesh"));
	IronChainMesh->SetupAttachment(PillarMesh);
	IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    IronChain1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChain1"));
    IronChain1->SetupAttachment(IronChainMesh);
    IronChain1->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    IronChain2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChain2"));
    IronChain2->SetupAttachment(IronChainMesh);
    IronChain2->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Wheel Attach
	HandWheelHole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelHole"));
	HandWheelHole->SetupAttachment(PillarMesh);
	HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    // Oil applied
	HandWheelLubricantPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelLubricantPoint"));
	HandWheelLubricantPoint->SetupAttachment(PillarMesh);
	HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	IronDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronDoorMesh"));
	IronDoorMesh->SetupAttachment(Root);
	IronDoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	CurrentLockPhase = E_LockPhase::E_ChainLock;

    bDoorAutoClose = false;
    bDoorForceOpen = false;
    DoorAutoCloseSpeed = 6.0f;
}

void APGExitIronDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGExitIronDoor, CurrentDoorHeight);
    DOREPLIFETIME(APGExitIronDoor, CurrentWheelQuat);
}

void APGExitIronDoor::HighlightOn() const
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            IronChainMesh->SetRenderCustomDepth(true);
            IronChain1->SetRenderCustomDepth(true);
            IronChain2->SetRenderCustomDepth(true);
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
            UE_LOG(LogPGExitPoint, Error, TEXT(""));
            break;
        }
    }
}

void APGExitIronDoor::HighlightOff() const
{
    IronChainMesh->SetRenderCustomDepth(false);
    IronChain1->SetRenderCustomDepth(false);
    IronChain2->SetRenderCustomDepth(false);
    HandWheelHole->SetRenderCustomDepth(false);
    HandWheelLubricantPoint->SetRenderCustomDepth(false);
}

FInteractionInfo APGExitIronDoor::GetInteractionInfo() const
{
    switch (CurrentLockPhase)
    {
        case E_LockPhase::E_ChainLock:
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("Unlock chain"));

            break;
        }
        case E_LockPhase::E_WheelAttach:
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("Attach Wheel"));

            break;
        }
        case E_LockPhase::E_OilApplied:
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("Oiled Wheel"));

            break;
        }
        case E_LockPhase::E_Unlocked:
        {
            // MaxDoorHeight
            // CurrentDoorHeight
            // 5.0f * ( MaxDoorHeight - CurrentDoorHeight ) / MaxDoorHeight
            const float UnlockDuration = 5.0f;
            const float UnlockStartTime = 5.0f * CurrentDoorHeight / MaxDoorHeight;

            UE_LOG(LogPGExitPoint, Log, TEXT("ExitIronDoor Unlock Duration : [%f], Unlock Start Time : [%f]"), UnlockDuration, UnlockStartTime);

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
            UE_LOG(LogPGExitPoint, Log, TEXT("CanStartInteraction ChainLock"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Chain is locked"));

        return false;
    }
    case E_LockPhase::E_WheelAttach:
    {
        if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.Wheel"))))
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("CanStartInteraction WheelPoint"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Required Wheel"));

        return false;
    }
    case E_LockPhase::E_OilApplied:
    {
        if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Exit.RustOil"))))
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("CanStartInteraction HandWheel"));
            return true;
        }
        OutFailureMessage = FText::FromString(TEXT("Required Oil to remove rust"));

        return false;
    }
    case E_LockPhase::E_Unlocked:
    {
        if (bDoorForceOpen)
        {
            return false;
        }

        UE_LOG(LogPGExitPoint, Log, TEXT("E_Unlocked"));
        break;
    }
    }
	return true;
}

void APGExitIronDoor::InteractionFailed()
{
    TArray<UMaterialInstanceDynamic*> MIDsToShake;
    switch (CurrentLockPhase)
    {
    case E_LockPhase::E_ChainLock:
    {
        MIDsToShake.Add(MIDChainLock);
        MIDsToShake.Add(MIDIronChain1);
        MIDsToShake.Add(MIDIronChain2);

        Multicast_ActivateShakeEffect(MIDsToShake);

        // NeedSound : 철그럭 거리는 자물쇠 실패 소리

        break;
    }
    case E_LockPhase::E_OilApplied:
    {
        MIDsToShake.Add(MIDWheel);

        Multicast_ActivateShakeEffect(MIDsToShake);

        break;

        // NeedSound : 녹슨거 돌리려다가 안돌아가는 끼릭 거리는 소리
    }
    }
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
        CurrentWheelQuat = AdditiveRotation * CurrentQuat;

        // 4. 새로운 쿼터니언 회전을 Rotator로 변환하여 설정합니다.
        HandWheelLubricantPoint->SetRelativeRotation(CurrentWheelQuat.Rotator());

        // Door open
        if (bDoorAutoClose)
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("Set bDoorAutoClose False"));
            bDoorAutoClose = false;
            SetActorTickEnabled(false);
        }
        CurrentDoorHeight = Progress * MaxDoorHeight;
        const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
        IronDoorMesh->SetRelativeLocation(NewDoorLocation);

        // 강제 셧다운 조작 시 닫히는 속도 원상복구.
        DoorAutoCloseSpeed = 6.0f;

        // NeedSound : 핸들 돌리는 소리, 철문 올라가는 소리.
    }
}

void APGExitIronDoor::StopHoldProress()
{
    UE_LOG(LogPGExitPoint, Log, TEXT("Exit Iron Door stop hold progress"));
    if (CurrentDoorHeight > 0)
    {
        UE_LOG(LogPGExitPoint, Log, TEXT("Set bDoorAutoClose True"));
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

            Multicast_UnlockChains();

            return true;
        }
        case E_LockPhase::E_WheelAttach:
        {
            CurrentLockPhase = E_LockPhase::E_OilApplied;

            Multicast_AttachWheel();

            return true;
        }
        case E_LockPhase::E_OilApplied:
        {
            CurrentLockPhase = E_LockPhase::E_Unlocked;
            Multicast_SetWheelMaterialOiled();

            // NeedSound : 반짝? 삑? 기름 바르고 깔끔한 소리

            UE_LOG(LogPGExitPoint, Log, TEXT("Oiled Wheel"));

            return true;
        }
        case E_LockPhase::E_Unlocked:
        {
            ///
            /// TODO : Open Door Completed and stop
            /// NeedSound : 10초의 철컹철컹 타이머
            StopHoldProress();

            FTimerManager& TimerManager = GetWorldTimerManager();

            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(this, FName("DoorForceClose"));

            // 타이머 설정
            TimerManager.SetTimer(
                DoorForceOpenTimerHandle, 
                TimerDelegate,      
                10.0f, 
                false
            );

            bDoorForceOpen = true;

            break;
        }
    }

    return false;
}

void APGExitIronDoor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    // 스태틱 메시 컴포넌트에 할당된 머티리얼이 있는지 확인하고 MID 생성
    if (IronChainMesh->GetMaterial(0))
    {
        MIDChainLock = IronChainMesh->CreateDynamicMaterialInstance(0);
    }
    if (IronChain1->GetMaterial(0))
    {
        MIDIronChain1 = IronChain1->CreateDynamicMaterialInstance(0);
    }
    if (IronChain2->GetMaterial(0))
    {
        MIDIronChain2 = IronChain2->CreateDynamicMaterialInstance(0);
    }
    if (HandWheelLubricantPoint->GetMaterial(0))
    {
        MIDWheel = HandWheelLubricantPoint->CreateDynamicMaterialInstance(0);
    }

    // set base start door location
    Multicast_SetDoorBaseLocation();
}

void APGExitIronDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // Close Door Automatically
    if (bDoorAutoClose && !bDoorForceOpen)
    {
        CurrentDoorHeight -= DeltaSeconds * DoorAutoCloseSpeed;

        const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
        IronDoorMesh->SetRelativeLocation(NewDoorLocation);

        // // NeedSound : 철문 떨어지는 소리. 속도 따라 다르게 해야하는ㄷ ㅔ이거....

        if (CurrentDoorHeight <= 0)
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("The iron door closed automatically"));

            //  NeedSound : 쾅 닫히는 소리

            CurrentDoorHeight = 0;
            SetActorTickEnabled(false);
            bDoorAutoClose = false;
            DoorAutoCloseSpeed = 6.0f;
        }
    }
}

void APGExitIronDoor::Multicast_SetDoorBaseLocation_Implementation()
{
    DoorBaseLocation = IronDoorMesh->GetRelativeLocation();
}

void APGExitIronDoor::Multicast_UnlockChains_Implementation()
{
    IronChainMesh->SetSimulatePhysics(true);
    IronChainMesh->SetCollisionProfileName(TEXT("Item"));
    IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    IronChain1->SetSimulatePhysics(true);
    IronChain1->SetCollisionProfileName(TEXT("Item"));
    IronChain1->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    IronChain2->SetSimulatePhysics(true);
    IronChain2->SetCollisionProfileName(TEXT("Item"));
    IronChain2->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    UE_LOG(LogPGExitPoint, Log, TEXT("Unlock chain"));

    // NeedSound  : 자물쇠 해제 소리, 0.2 초 후 철소리( 체인 떨어지는 소리 ), 멀티캐스트 함수 내부라서 개인실행 해야 할듯?
}

void APGExitIronDoor::Multicast_AttachWheel_Implementation()
{
    HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    HandWheelLubricantPoint->SetVisibility(true);
    HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // // NeedSound  : 대충 끼는 소리

    UE_LOG(LogPGExitPoint, Log, TEXT("Attach Wheel"));
}

void APGExitIronDoor::Multicast_SetWheelMaterialOiled_Implementation()
{
    if (HandWheelOiledMaterial)
    {
        HandWheelLubricantPoint->SetMaterial(0, HandWheelOiledMaterial);
    }
}

void APGExitIronDoor::Multicast_ActivateShakeEffect_Implementation(const TArray<UMaterialInstanceDynamic*>& TargetMIDs)
{
    ToggleShakeEffect(TargetMIDs, true);
    
    // 0.5초 후 DisableEffect 함수를 호출하도록 타이머 설정 (TimerHandle1 관리)
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("DisableShakeEffect"), TargetMIDs);

    GetWorldTimerManager().SetTimer(
        ShakeEffectTimerHandle,
        TimerDelegate,
        0.1f,
        false
    );
}

void APGExitIronDoor::DisableShakeEffect(const TArray<UMaterialInstanceDynamic*>& TargetMIDs)
{
    ToggleShakeEffect(TargetMIDs, false);

    GetWorldTimerManager().ClearTimer(ShakeEffectTimerHandle);
}

void APGExitIronDoor::ToggleShakeEffect(const TArray<UMaterialInstanceDynamic*>& TargetMIDs, bool bToggle)
{
    float TargetValue = bToggle ? 1.0f : 0.0f;

    for (UMaterialInstanceDynamic* TargetMID : TargetMIDs)
    {
        if (TargetMID)
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("%s : ToggleShakeEffect %s"), *TargetMID->GetName(), bToggle ? TEXT("TRUE") : TEXT("FALSE"));

            TargetMID->SetScalarParameterValue(TargetParameterName, TargetValue);
        }
        else
        {
            UE_LOG(LogPGExitPoint, Warning, TEXT("APGExitIronDoor::ToggleShakeEffect - Null TargetMID found in array."));
        }
    }
}

void APGExitIronDoor::OnRep_CurrentDoorHeight()
{
    const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
    IronDoorMesh->SetRelativeLocation(NewDoorLocation);
}

void APGExitIronDoor::OnRep_CurrentWheelQuat()
{
    HandWheelLubricantPoint->SetRelativeRotation(CurrentWheelQuat.Rotator());
}

void APGExitIronDoor::DoorForceClose()
{
    bDoorForceOpen = false;
    DoorAutoCloseSpeed = 250.0f;

    GetWorldTimerManager().ClearTimer(DoorForceOpenTimerHandle);

    // NeedSound : 탕 하고 철로된 뭔가가 끊어지는 소리.

    UE_LOG(LogPGExitPoint, Log, TEXT("Start Door force close."));
}

/*
oiled unlock -> StopHoldProgress true
hold to unhold -> StopHoldProgress true

hold progress -> bDoorAutoClose false

tick -> 높이 일정수치 미만이면 종료

강제 오픈 함수 추가.
*/