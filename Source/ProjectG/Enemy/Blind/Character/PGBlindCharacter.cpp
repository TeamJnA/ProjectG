// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"

APGBlindCharacter::APGBlindCharacter()
{
    BlindAttributeSet = CreateDefaultSubobject<UPGBlindAttributeSet>("BlindAttributeSet");

    

    
    BiteCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BiteCollider"));
    BiteCollider->SetupAttachment(GetMesh());
    BiteCollider->SetBoxExtent(FVector(50.f)); // �⺻ ũ�� (BP���� ���� ����)
    BiteCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BiteCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    BiteCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    
    static ConstructorHelpers::FObjectFinder<UAnimMontage> BiteMontageRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/Animation/Fight/AM_BlindBite.AM_BlindBite"));
    if (BiteMontageRef.Succeeded())
    {
        BiteMontage = BiteMontageRef.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> RoarMontageRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/Animation/Other/AM_BlindRoar.AM_BlindRoar"));
    if (RoarMontageRef.Succeeded())
    {
        RoarMontage = RoarMontageRef.Object;
    }

}

float APGBlindCharacter::GetExplorationRadius() const
{
    return ExplorationRadius;
}

float APGBlindCharacter::GetExplorationWaitTime() const
{
    return ExplorationWaitTime;
}



float APGBlindCharacter::GetNoiseLevelThreshold() const
{
    return NoiseLevelThreshold;
}

float APGBlindCharacter::GetNoiseMaxThreshold() const
{
    return NoiseMaxThreshold;
}

int APGBlindCharacter::GetHuntLevel() const
{
    return HuntLevel;
}

void APGBlindCharacter::SetHuntLevel(int Level)
{
    if (HasAuthority())
    {
        HuntLevel = Level;
    }
    
}

void APGBlindCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGBlindCharacter, HuntLevel);
}


void APGBlindCharacter::BeginPlay()
{
    Super::BeginPlay();

    //collider ���̱�.
    if (BiteCollider && GetMesh())
    {
        BiteCollider->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Jaw1Socket"));
    }


    //callback �Լ�  ���
    if (BiteCollider)
    {
        BiteCollider->OnComponentBeginOverlap.AddDynamic(this, &APGBlindCharacter::OnBiteColliderOverlapBegin);
    }


}



void APGBlindCharacter::OnBiteColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this)
        return;
    
    // �÷��̾ ���� (�ʿ� �� �ٸ� ���� �߰�)
    // ����� tag ������� �۵�. �α� ��� �ٸ� ��� �߰��ϸ� ��.
    if (OtherActor->ActorHasTag(FName("Player")))
    {
        UE_LOG(LogTemp, Log, TEXT(" Bite Successful"));
    }
    
    

}
