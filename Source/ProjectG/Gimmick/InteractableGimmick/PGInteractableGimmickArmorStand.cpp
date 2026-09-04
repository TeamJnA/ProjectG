// Fill out your copyright notice in the Description page of Project Settings.

#include "PGInteractableGimmickArmorStand.h"
#include "Components/BoxComponent.h"
#include "Item/ItemActor/PGProjectileItemBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/PGSoundManager.h"
#include "PGLogChannels.h"


APGInteractableGimmickArmorStand::APGInteractableGimmickArmorStand()
{
    SetNetUpdateFrequency(30.0f);

    ArmorBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ArmorBoxCollision"));
    ArmorBoxCollision->SetupAttachment(RootComponent);

    CameraShakeSource = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("CameraShakeSource"));
    CameraShakeSource->SetupAttachment(RootComponent);
}

void APGInteractableGimmickArmorStand::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGInteractableGimmickArmorStand, bCollapsed);
    DOREPLIFETIME(APGInteractableGimmickArmorStand, CollapsedState);
}

void APGInteractableGimmickArmorStand::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        ArmorBoxCollision->OnComponentHit.AddDynamic(this, &APGInteractableGimmickArmorStand::OnHit);
    }
}

void APGInteractableGimmickArmorStand::PostInitializeComponents()
{
	Super::PostInitializeComponents();

    ArmorMeshs.Reset();

	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);

    for (UStaticMeshComponent* Mesh : StaticMeshes)
    {
        if (Mesh && Mesh->ComponentHasTag(TEXT("Armor")))
        {
            ArmorMeshs.Add(Mesh);
            Mesh->SetSimulatePhysics(false);
        }
    }

    ArmorMeshs.Sort([](const UStaticMeshComponent& A, const UStaticMeshComponent& B)
        {
            return A.GetFName().LexicalLess(B.GetFName());
        });
}

void APGInteractableGimmickArmorStand::GimmickInteract(AActor* Investigator)
{
    if (bCollapsed)
    {
        return;
    }

    CollapseArmor(Investigator);
}

void APGInteractableGimmickArmorStand::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bCollapsed)
    {
        return;
    }

    if (!IsValid(OtherActor))
    {
        return;
    }

    // Check it is brick or player character
    // APGProjectileItemBase     APGPlayerCharacter
    UE_LOG(LogPGInteractableGimmick, Log, TEXT("ArmorStand OnHit"));

    if (OtherActor->IsA<APGProjectileItemBase>() || OtherActor->IsA<APGPlayerCharacter>())
    {
        UE_LOG(LogPGInteractableGimmick, Log, TEXT("ArmorStand OnHit by proper actor"));

        CollapseArmor(OtherActor->GetInstigator());
    }
}

void APGInteractableGimmickArmorStand::CollapseArmor(AActor* Investigator)
{
    if (!HasAuthority() || bCollapsed)
    {
        return;
    }

    bCollapsed = true;
    ApplyCollapseState();

    // Play Sound
    if (APGSoundManager* SM = GetSoundManager())
    {
        SM->PlaySoundWithNoise(ArmorStandCollapseSound, GetActorLocation(), false, Investigator);
    }
    else
    {
        UE_LOG(LogPGInteractableGimmick, Warning, TEXT("ArmorStand: No SoundManager"));
    }

    GetWorldTimerManager().SetTimer(SettleTimerHandle, this,
        &APGInteractableGimmickArmorStand::CaptureSettledTransforms, 3.0f, false);
}

void APGInteractableGimmickArmorStand::CaptureSettledTransforms()
{
    CollapsedState.Reset(ArmorMeshs.Num());

    // 물리를 끄면 absolute 상태가 풀려 서버 조각들이 튐
    // => 끈 직후 위치 고정
    for (UStaticMeshComponent* M : ArmorMeshs)
    {
        FPGArmorPieceState S;
        S.Location = M->GetComponentLocation();
        S.Rotation = M->GetComponentRotation();
        CollapsedState.Add(S);

        M->SetSimulatePhysics(false);
        M->SetWorldLocationAndRotation(S.Location, S.Rotation,
            false, nullptr, ETeleportType::TeleportPhysics);
    }

    bLocalSettleApplied = true;
    SetNetDormancy(DORM_DormantAll);
}

void APGInteractableGimmickArmorStand::OnRep_Collapsed()
{
    ApplyCollapseState();
}

void APGInteractableGimmickArmorStand::OnRep_CollapsedState()
{
    ApplyCollapseState();
}

void APGInteractableGimmickArmorStand::ApplyCollapseState()
{
    if (!bCollapsed)
    {
        return;
    }

    ArmorBoxCollision->SetCollisionProfileName(TEXT("NoCollision"));
    SelfHighlightOff();

    if (ArmorMeshs.Num() == 0)
    {
        UE_LOG(LogPGInteractableGimmick, Error, TEXT("[%s] No meshes tagged 'Armor'. Collapse do nothing."), *GetName());
        return;
    }

    // 붕괴 완료 후 서버에서 위치 확정 완료
    // 위치 맞추기, collision 세팅 만 하면 됨
    const bool bSettled = (CollapsedState.Num() == ArmorMeshs.Num());
    if (bSettled)
    {
        if (bLocalSettleApplied)
        {
            return;
        }
        bLocalSettleApplied = true;

        for (int32 i = 0; i < ArmorMeshs.Num(); ++i)
        {
            UStaticMeshComponent* M = ArmorMeshs[i];
            M->SetSimulatePhysics(false);
            M->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            M->SetCollisionResponseToChannel(ECC_FootStep, ECR_Block);
            M->SetWorldLocationAndRotation(CollapsedState[i].Location, CollapsedState[i].Rotation,
                false, nullptr, ETeleportType::TeleportPhysics);
        }
        return;
    }

    // 붕괴 완료 전 위치 확정 x
    // physics 시뮬
    if (bLocalPhysicsStarted)
    {
        return;
    }
    bLocalPhysicsStarted = true;

    for (UStaticMeshComponent* M : ArmorMeshs)
    {
        M->SetSimulatePhysics(true);
        M->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        M->SetCollisionResponseToChannel(ECC_FootStep, ECR_Block);
    }

    CameraShakeSource->Start();
}

void APGInteractableGimmickArmorStand::HighlightOn() const
{
}

void APGInteractableGimmickArmorStand::HighlightOff() const
{
}

void APGInteractableGimmickArmorStand::SelfHighlightOff()
{
    for (UStaticMeshComponent* ArmorMesh : ArmorMeshs)
    {
        if (ArmorMesh)
        {
            ArmorMesh->SetRenderCustomDepth(false);
        }
    }
}

FInteractionInfo APGInteractableGimmickArmorStand::GetInteractionInfo() const
{
    return FInteractionInfo(EInteractionType::Instant);
}

FText APGInteractableGimmickArmorStand::GetInteractionText() const
{
    return BreakText;
}
