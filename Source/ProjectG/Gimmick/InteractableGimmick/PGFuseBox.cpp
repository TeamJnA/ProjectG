// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGFuseBox.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"
#include "Interface/LightEffectInterface.h"

#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"


// Sets default values
APGFuseBox::APGFuseBox()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(Root);

    CoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverMesh"));
    CoverMesh->SetupAttachment(Root);

    FuseAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("FuseAttachPoint"));
    FuseAttachPoint->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void APGFuseBox::BeginPlay()
{
	Super::BeginPlay();

    MIDCover = CoverMesh->CreateDynamicMaterialInstance(0);
}

void APGFuseBox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGFuseBox, FuseBoxState);
    DOREPLIFETIME(APGFuseBox, OwnerRoom);
    DOREPLIFETIME(APGFuseBox, ShakeStep);
}

TSubclassOf<UGameplayAbility> APGFuseBox::GetAbilityToInteract() const
{
    return InteractAbility;
}

void APGFuseBox::HighlightOn() const
{
}

void APGFuseBox::HighlightOff() const
{
}

FInteractionInfo APGFuseBox::GetInteractionInfo() const
{
    return FInteractionInfo(EInteractionType::Hold, 10.0f);
}

bool APGFuseBox::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
    if (FuseBoxState != EFuseBoxState::Closed)
    {
        OutFailureMessage = FText::FromString(TEXT("Already opened"));
        return false;
    }
    return true;
}

void APGFuseBox::OpenBox()
{
    if (!HasAuthority() || FuseBoxState != EFuseBoxState::Closed)
    {
        return;
    }

    //Play Sound
    if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
    {
        if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
        {
            FVector SoundPlayLocation = GetActorLocation() - FVector::ZAxisVector * (-200);
            SoundManager->PlaySoundWithNoise(CoverFallSound, SoundPlayLocation);
        }
    }

    FuseBoxState = EFuseBoxState::Opened;
    OnRep_FuseBoxState();

    SpawnFuseItem();
}

void APGFuseBox::SpawnFuseItem()
{
    UPGItemData* FuseData = FuseItemDataPath.LoadSynchronous();
    if (!FuseData)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    const FTransform SpawnTransform = FuseAttachPoint->GetComponentTransform();

    SpawnedFuseItem = GetWorld()->SpawnActor<APGItemActor>(APGItemActor::StaticClass(), SpawnTransform, SpawnParams);
    if (SpawnedFuseItem)
    {
        SpawnedFuseItem->InitWithData(FuseData);
        SpawnedFuseItem->OnDestroyed.AddDynamic(this, &APGFuseBox::OnFuseItemDestroyed);
    }
}

void APGFuseBox::OnFuseItemDestroyed(AActor* DestroyedActor)
{
    if (!HasAuthority())
    {
        return;
    }

    //Play Sound
    if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
    {
        if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
        {
            SoundManager->PlaySoundForAllPlayers(FuseTakeSound, GetActorLocation());
        }
    }

    FuseBoxState = EFuseBoxState::Empty;
    OnRep_FuseBoxState();
}

void APGFuseBox::OnRep_FuseBoxState()
{
    switch (FuseBoxState)
    {
        case EFuseBoxState::Opened:
        {
            if (MIDCover)
            {
                MIDCover->SetScalarParameterValue(ShakeParameterName, 0.0f);
            }

            BodyMesh->SetRenderCustomDepth(false);
            BodyMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            BodyMesh->SetGenerateOverlapEvents(false);

            CoverMesh->SetRenderCustomDepth(false);
            CoverMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            CoverMesh->SetGenerateOverlapEvents(false);

            // Simulate physics
            CoverMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            CoverMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
            CoverMesh->SetCollisionObjectType(ECC_PhysicsBody);
            CoverMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            CoverMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Ignore);
            CoverMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);
            CoverMesh->SetSimulatePhysics(true);
            FVector Direction = (GetActorForwardVector() - GetActorRightVector() - FVector(0, 0, 0.3f)).GetSafeNormal();
            CoverMesh->AddImpulse(Direction * 200.0f, NAME_None, true);

            break;
        }

        case EFuseBoxState::Empty:
        {
            TurnOffRoomLights();
            break;
        }

        default:
        {
            break;
        }
    }
}

void APGFuseBox::TurnOffRoomLights()
{
    if (!OwnerRoom)
    {
        return;
    }

    TArray<UActorComponent*> Components;
    OwnerRoom->GetComponents(Components);
    for (UActorComponent* Comp : Components)
    {
        if (ILightEffectInterface* LightEffect = Cast<ILightEffectInterface>(Comp))
        {
            LightEffect->PowerOff();
        }
    }
}

void APGFuseBox::UpdateHoldProgress(float Progress)
{
    uint8 NewStep = 0;
    if (Progress >= 0.9f)
    {
        NewStep = 3;
    }
    else if (Progress >= 0.6f)
    {
        NewStep = 2;
    }
    else if (Progress >= 0.3f)
    {
        NewStep = 1;
    }

    if (NewStep > ShakeStep)
    {
        if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
        {
            if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
            {
                SoundManager->PlaySoundWithNoise(CoverShakeSound, GetActorLocation());
            }
        }

        ShakeStep = NewStep;
        OnRep_ShakeStep();
    }
}

void APGFuseBox::OnRep_ShakeStep()
{
    if (ShakeStep > 0 && MIDCover)
    {
        float WPOPowerValue = 1.0f;
        switch (ShakeStep)
        {
            case 1:
            {
                WPOPowerValue = 1.0f;
                break;
            }

            case 2:
            {
                WPOPowerValue = 1.5f;
                break;
            }

            case 3:
            {
                WPOPowerValue = 2.0f;
                break;
            }
        }

        MIDCover->SetScalarParameterValue(ShakeParameterName, WPOPowerValue);
        GetWorldTimerManager().SetTimer(ShakeEffectTimerHandle, this, &APGFuseBox::DisableShakeEffect, 0.1f, false);
    }
}

void APGFuseBox::DisableShakeEffect()
{
    if (MIDCover)
    {
        MIDCover->SetScalarParameterValue(ShakeParameterName, 0.0f);
    }
}

void APGFuseBox::StopHoldProress()
{
    ShakeStep = 0;
    if (MIDCover)
    {
        MIDCover->SetScalarParameterValue(ShakeParameterName, 0.0f);
    }
    GetWorldTimerManager().ClearTimer(ShakeEffectTimerHandle);
}
