// Fill out your copyright notice in the Description page of Project Settings.

#include "PGInteractableGimmickArmorStand.h"
#include "Components/BoxComponent.h"
#include "Item/ItemActor/PGProjectileItemBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Net/UnrealNetwork.h"
#include "PGLogChannels.h"

APGInteractableGimmickArmorStand::APGInteractableGimmickArmorStand()
{
    ArmorBoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ArmorBoxCollision"));
    ArmorBoxCollision->SetupAttachment(RootComponent);

    CameraShakeSource = CreateDefaultSubobject<UCameraShakeSourceComponent>(TEXT("CameraShakeSource"));
    CameraShakeSource->SetupAttachment(RootComponent);

    bIsCollisionDisabled = false;
}

void APGInteractableGimmickArmorStand::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGInteractableGimmickArmorStand, bIsCollisionDisabled);
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

	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);

    for (UStaticMeshComponent* Mesh : StaticMeshes)
    {
        if (Mesh && Mesh->ComponentHasTag(TEXT("Armor")))
        {
            ArmorMeshs.Add(Mesh);

            Mesh->SetSimulatePhysics(false);

            Mesh->SetIsReplicated(true);
        }
    }
}

void APGInteractableGimmickArmorStand::GimmickInteract()
{
    CollapseArmor();
}

void APGInteractableGimmickArmorStand::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Check it is brick or player character
    // APGProjectileItemBase     APGPlayerCharacter
    UE_LOG(LogPGInteractableGimmick, Log, TEXT("ArmorStand OnHit"));

    if (OtherActor->IsA<APGProjectileItemBase>() || OtherActor->IsA<APGPlayerCharacter>())
    {
        UE_LOG(LogPGInteractableGimmick, Log, TEXT("ArmorStand OnHit by proper actor"));

        CollapseArmor();
    }
}

void APGInteractableGimmickArmorStand::OnRep_CollisionDisabled()
{
    ArmorBoxCollision->SetCollisionProfileName(TEXT("NoCollision"));
    CameraShakeSource->Start();
}

void APGInteractableGimmickArmorStand::CollapseArmor()
{
    UE_LOG(LogPGInteractableGimmick, Log, TEXT("Armor has collapsed."));

    for (UStaticMeshComponent* ArmorMesh : ArmorMeshs)
    {
        if (ArmorMesh)
        {
            ArmorMesh->SetSimulatePhysics(true);
            ArmorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        }
    }

    bIsCollisionDisabled = true;
    ArmorBoxCollision->SetCollisionProfileName(TEXT("NoCollision"));
    CameraShakeSource->Start();

    // Highlight ²ô±â
    HighlightOff();
}

void APGInteractableGimmickArmorStand::HighlightOn() const
{

    for (UStaticMeshComponent* ArmorMesh : ArmorMeshs)
    {
        if (ArmorMesh)
        {
            ArmorMesh->SetRenderCustomDepth(true);
            ArmorMesh->SetCustomDepthStencilValue(0);
        }
    }
}

void APGInteractableGimmickArmorStand::HighlightOff() const
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
