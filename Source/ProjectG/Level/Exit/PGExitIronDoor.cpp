// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitIronDoor.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "PGLogChannels.h"

#include "Character/PGPlayerCharacter.h"
#include "Game/PGGameMode.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"

APGExitIronDoor::APGExitIronDoor()
{
    PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PillarMesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh0"));
	PillarMesh0->SetupAttachment(Root);
	PillarMesh0->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    PillarMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh1"));
    PillarMesh1->SetupAttachment(Root);
    PillarMesh1->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);	
    
    PillarMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh2"));
    PillarMesh2->SetupAttachment(Root);
    PillarMesh2->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    
    PillarMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh3"));
    PillarMesh3->SetupAttachment(Root);
    PillarMesh3->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    // Chain Lock
	IronChainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChainMesh"));
	IronChainMesh->SetupAttachment(PillarMesh0);
	IronChainMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    IronChain1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChain1"));
    IronChain1->SetupAttachment(IronChainMesh);
    IronChain1->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    IronChain2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronChain2"));
    IronChain2->SetupAttachment(IronChainMesh);
    IronChain2->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Wheel Attach
	HandWheelHole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelHole"));
	HandWheelHole->SetupAttachment(PillarMesh0);
	HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    // Oil applied
	HandWheelLubricantPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandWheelLubricantPoint"));
	HandWheelLubricantPoint->SetupAttachment(PillarMesh0);
	HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	IronDoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IronDoorMesh"));
	IronDoorMesh->SetupAttachment(Root);
	IronDoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    IronDoorMesh->bFillCollisionUnderneathForNavmesh = true;

    IronDoorSoundPlayOffset = CreateDefaultSubobject<USceneComponent>(TEXT("IronDoorSoundPlayOffset"));
    IronDoorSoundPlayOffset->SetupAttachment(Root);

    // Escape Trigger Volume
    EscapeTriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTriggerVolume"));
    EscapeTriggerVolume->SetupAttachment(Root);
    EscapeTriggerVolume->SetRelativeLocation(FVector(-69.0f, -154.0f, 153.0f));
    EscapeTriggerVolume->SetBoxExtent(FVector(250.0f, 210.0f, 153.0f));
    EscapeTriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    EscapeTriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
    EscapeTriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    EscapeTriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	CurrentLockPhase = E_LockPhase::E_ChainLock;

    bDoorAutoClose = false;
    bDoorForceOpen = false;
    DoorAutoCloseSpeed = 6.0f;

    bIsChain = true;

    SoundPlayChecker.Init(false, 21);
}

void APGExitIronDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGExitIronDoor, CurrentDoorHeight);
    DOREPLIFETIME(APGExitIronDoor, CurrentWheelQuat);
    DOREPLIFETIME(APGExitIronDoor, CurrentLockPhase);
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
    switch (CurrentLockPhase)
    {
    case E_LockPhase::E_ChainLock:
    {
        Multicast_ActivateShakeEffect();

        PlaySound(CannotUnlockChainSound, IronChainMesh->GetComponentLocation());

        break;
    }
    case E_LockPhase::E_OilApplied:
    {
        Multicast_ActivateShakeEffect();

        PlaySound(CannotRotateWheelSound, HandWheelLubricantPoint->GetComponentLocation());

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
        FQuat CurrentQuat = HandWheelLubricantPoint->GetRelativeRotation().Quaternion();
        FQuat AdditiveRotation = FQuat(FVector::YAxisVector, FMath::DegreesToRadians(1.0f));
        CurrentWheelQuat = AdditiveRotation * CurrentQuat;
        HandWheelLubricantPoint->SetRelativeRotation(CurrentWheelQuat.Rotator());

        // Door open
        if (bDoorAutoClose)
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("Set bDoorAutoClose False"));
            bDoorAutoClose = false;
            SetActorTickEnabled(false);
            CleanSoundChecker();

            // 강제 셧다운 조작 시 닫히는 속도 원상복구. 
            DoorAutoCloseSpeed = 6.0f;
        }
        CurrentDoorHeight = Progress * MaxDoorHeight;
        const FVector NewDoorLocation = DoorBaseLocation + FVector(0.0f, 0.0f, CurrentDoorHeight);
        IronDoorMesh->SetRelativeLocation(NewDoorLocation);

        // Play Sound
        int32 NowIndex = ((float)CurrentDoorHeight / MaxDoorHeight) / 0.05f;
        if (!SoundPlayChecker[NowIndex])
        {
            SoundPlayChecker[NowIndex] = true;
            // PlaySound(IronDoorMeshBaseSound, IronDoorMesh->GetComponentLocation());

            if (NowIndex % 4 == 0 && NowIndex != 20)
            {
                PlaySound(IronDoorMeshRustySound, IronDoorSoundPlayOffset->GetComponentLocation());
            }

            if (NowIndex % 2 == 0 && NowIndex != 20)
            {
                PlaySound(WheelRotateRustySound, HandWheelLubricantPoint->GetComponentLocation());
            }
        }
    }
}

void APGExitIronDoor::StopHoldProress()
{
    UE_LOG(LogPGExitPoint, Log, TEXT("Exit Iron Door stop hold progress"));
    if (CurrentDoorHeight > 0)
    {
        UE_LOG(LogPGExitPoint, Log, TEXT("Set bDoorAutoClose True"));
        bDoorAutoClose = true;
        CleanSoundChecker();
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

            PlaySound(UnlockChainSound, IronChainMesh->GetComponentLocation());

            // Play drop chain sound
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(this, FName("PlayChainDropSound"));

            GetWorldTimerManager().SetTimer(
                ChainDropTimerHandle,
                TimerDelegate,
                0.4f,
                false
            );

            return true;
        }
        case E_LockPhase::E_WheelAttach:
        {
            CurrentLockPhase = E_LockPhase::E_OilApplied;

            Multicast_AttachWheel();

            PlaySound(WheelAttachedSound, HandWheelLubricantPoint->GetComponentLocation());

            return true;
        }
        case E_LockPhase::E_OilApplied:
        {
            CurrentLockPhase = E_LockPhase::E_Unlocked;
            Multicast_SetWheelMaterialOiled();

            PlaySound(OilAppliedSound, HandWheelLubricantPoint->GetComponentLocation());

            UE_LOG(LogPGExitPoint, Log, TEXT("Unlock :: case E_OilApplied"));

            return true;
        }
        case E_LockPhase::E_Unlocked:
        {
            StopHoldProress();

            FTimerManager& TimerManager = GetWorldTimerManager();

            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(this, FName("DoorForceClose"));

            // 타이머 설정 및 카운트다운 오디오 시작
            TimerManager.SetTimer(
                DoorForceOpenTimerHandle, 
                TimerDelegate,      
                10.0f, 
                false
            );

            bDoorForceOpen = true;

            Multicast_StartCloseCountSound();

            break;
        }
    }

    return false;
}

void APGExitIronDoor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    InitMIDs();

    // set base start door location
    DoorBaseLocation = IronDoorMesh->GetRelativeLocation();

    if (HasAuthority())
    {
        EscapeTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APGExitIronDoor::OnEscapeTriggerOverlap);
    }
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
        int32 NowIndex = ((float)CurrentDoorHeight / MaxDoorHeight) / 0.05f;
        if (!SoundPlayChecker[NowIndex])
        {
            SoundPlayChecker[NowIndex] = true;
            if (NowIndex % 2 == 0 && NowIndex != 0)
            {
                PlaySound(IronDoorMeshBaseSound, IronDoorSoundPlayOffset->GetComponentLocation());
            }
        }

        if (CurrentDoorHeight <= 0)
        {
            UE_LOG(LogPGExitPoint, Log, TEXT("The iron door closed automatically"));

            if (DoorAutoCloseSpeed > 10)
            {
                PlaySound(DoorClosedSound, IronDoorSoundPlayOffset->GetComponentLocation());
            }
            else
            {
                PlaySound(IronDoorMeshBaseSound, IronDoorSoundPlayOffset->GetComponentLocation());
            }

            CurrentDoorHeight = 0;
            SetActorTickEnabled(false);
            bDoorAutoClose = false;
            DoorAutoCloseSpeed = 6.0f;
            CleanSoundChecker();
        }
    }
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

    bIsChain = false;

    HighlightOff();

    UE_LOG(LogPGExitPoint, Log, TEXT("Unlock chain"));
}

void APGExitIronDoor::Multicast_AttachWheel_Implementation()
{
    HandWheelHole->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    HandWheelLubricantPoint->SetVisibility(true);
    HandWheelLubricantPoint->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    HighlightOff();

    UE_LOG(LogPGExitPoint, Log, TEXT("Attach Wheel"));
}

void APGExitIronDoor::Multicast_SetWheelMaterialOiled_Implementation()
{
    if (HandWheelOiledMaterial)
    {
        HandWheelLubricantPoint->SetMaterial(0, HandWheelOiledMaterial);
    }

    HighlightOff();

    UE_LOG(LogPGExitPoint, Log, TEXT("Wheel Oil Applied"));
}

void APGExitIronDoor::Multicast_ActivateShakeEffect_Implementation()
{
    if (ChainsToShake.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActivateShakeEffect : TargetMIDs is Null."));
        return;
    }

    ToggleShakeEffect(true);

    // 0.1초 후 DisableEffect 함수를 호출하도록 타이머 설정 (TimerHandle1 관리)
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUFunction(this, FName("DisableShakeEffect"));

    GetWorldTimerManager().SetTimer(
        ShakeEffectTimerHandle,
        TimerDelegate,
        0.1f,
        false
    );
}

void APGExitIronDoor::DisableShakeEffect()
{
    ToggleShakeEffect(false);

    GetWorldTimerManager().ClearTimer(ShakeEffectTimerHandle);
}

void APGExitIronDoor::ToggleShakeEffect(bool bToggle)
{
    float TargetValue = bToggle ? 1.0f : 0.0f;

    if (bIsChain)
    {
        for (UMaterialInstanceDynamic* TargetMID : ChainsToShake)
        {
            if (TargetMID)
            {
                UE_LOG(LogPGExitPoint, Log, TEXT("%s : ToggleShakeEffect %s"), *TargetMID->GetName(), bToggle ? TEXT("TRUE") : TEXT("FALSE"));

                TargetMID->SetScalarParameterValue(TargetParameterName, TargetValue);
            }
        }
    }
    else
    {
        for (UMaterialInstanceDynamic* TargetMID : WheelToShake)
        {
            if (TargetMID)
            {
                UE_LOG(LogPGExitPoint, Log, TEXT("%s : ToggleShakeEffect %s"), *TargetMID->GetName(), bToggle ? TEXT("TRUE") : TEXT("FALSE"));

                TargetMID->SetScalarParameterValue(TargetParameterName, TargetValue);
            }
        }
    }
}

void APGExitIronDoor::InitMIDs()
{
    if (IronChainMesh && !MIDChainLock)
    {
        if (IronChainMesh->GetMaterial(0))
        {
            MIDChainLock = IronChainMesh->CreateDynamicMaterialInstance(0);
            if (MIDChainLock)
            {
                ChainsToShake.AddUnique(MIDChainLock);
            }
        }
    }

    if (IronChain1 && !MIDIronChain1)
    {
        if (IronChain1->GetMaterial(0))
        {
            MIDIronChain1 = IronChain1->CreateDynamicMaterialInstance(0);
            if (MIDIronChain1)
            {
                ChainsToShake.AddUnique(MIDIronChain1);
            }
        }
    }

    if (IronChain2 && !MIDIronChain2)
    {
        if (IronChain2->GetMaterial(0))
        {
            MIDIronChain2 = IronChain2->CreateDynamicMaterialInstance(0);
            if (MIDIronChain2)
            {
                ChainsToShake.AddUnique(MIDIronChain2);
            }
        }
    }

    if (HandWheelLubricantPoint && !MIDWheel)
    {
        if (HandWheelLubricantPoint->GetMaterial(0))
        {
            MIDWheel = HandWheelLubricantPoint->CreateDynamicMaterialInstance(0);
            if (MIDWheel)
            {
                WheelToShake.AddUnique(MIDWheel);
            }
        }
    }

    UE_LOG(LogPGExitPoint, Log, TEXT("InitMIDs : Chains(%d), Wheels(%d)"), ChainsToShake.Num(), WheelToShake.Num());
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

    Multicast_StopCloseCountSound();

    PlaySound(DoorCloseStartSound, IronDoorSoundPlayOffset->GetComponentLocation());

    UE_LOG(LogPGExitPoint, Log, TEXT("Start Door force close."));
}

void APGExitIronDoor::CleanSoundChecker()
{
    SoundPlayChecker.Init(false, 21);
}

void APGExitIronDoor::PlayChainDropSound()
{
    PlaySound(ChainDropSound, IronChainMesh->GetComponentLocation());
}

void APGExitIronDoor::Multicast_StartCloseCountSound_Implementation()
{
    if (CloseCountSoundCue)
    {
        CloseCountSoundAudioComponent = UGameplayStatics::SpawnSound2D(this, CloseCountSoundCue);
    }
}

void APGExitIronDoor::Multicast_StopCloseCountSound_Implementation()
{
    if (CloseCountSoundAudioComponent && CloseCountSoundAudioComponent->IsPlaying())
    {
        CloseCountSoundAudioComponent->Stop();
    }
}

/*
oiled unlock -> StopHoldProgress true
hold to unhold -> StopHoldProgress true

hold progress -> bDoorAutoClose false

tick -> 높이 일정수치 미만이면 종료

강제 오픈 함수 추가.
*/

/*
* 플레이어가 ExitDoor를 열고 트리거에 닿은 경우 종료처리
*/
void APGExitIronDoor::OnEscapeTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (CurrentLockPhase != E_LockPhase::E_Unlocked)
    {
        return;
    }

    if (APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(OtherActor))
    {
        // 컷신 -> 종료처리 -> 종료 카메라 뷰 변환 -> 스코어보드
        if (APGPlayerState* PS = PlayerCharacter->GetPlayerState<APGPlayerState>(); PS && !PS->HasFinishedGame())
        {
            if (APGGameMode* GM = GetWorld()->GetAuthGameMode<APGGameMode>())
            {
                GM->HandlePlayerEscaping(PlayerCharacter);
            }

            if (APGPlayerController* PC = Cast<APGPlayerController>(PlayerCharacter->GetController()))
            {
                PC->Client_StartEscapeSequence();
            }
        }
    }
}