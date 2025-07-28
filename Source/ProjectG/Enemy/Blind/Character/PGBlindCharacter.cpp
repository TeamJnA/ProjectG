// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"

#include "Level/PGDoor1.h"

APGBlindCharacter::APGBlindCharacter()
{
    BlindAttributeSet = CreateDefaultSubobject<UPGBlindAttributeSet>("BlindAttributeSet");
        
    BiteCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BiteCollider"));
    BiteCollider->SetupAttachment(GetMesh());
    BiteCollider->SetBoxExtent(FVector(50.f)); // 기본 크기 (BP에서 조정 가능)
    BiteCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BiteCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    BiteCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    DoorDetectCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorDetectCollider"));
    DoorDetectCollider->SetupAttachment(RootComponent);
    DoorDetectCollider->SetBoxExtent(FVector(50.f));
    DoorDetectCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DoorDetectCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    DoorDetectCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
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
        // 문 탐색 콜리더를 이용해서, 
        // Huntlevel에 따라 닿은 문을 부순다는 함수를 bind하고 unbind한다
        // NewHuntLevel(Level) > 0 && CurHuntLevel(HuntLevel) == 0, AddDynamic
        // NewHuntLevel(Level) == 0 && CurHuntLevel(HuntLevel) > 0, RemoveDynamic

        if (Level > 0 && HuntLevel == 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Hunt level become 1,2 : Add OnOpenDoorColliderOverlapBegin"))
            DoorDetectCollider->OnComponentBeginOverlap.AddDynamic(this, &APGBlindCharacter::OnOpenDoorColliderOverlapBegin);
        }

        if (Level == 0 && HuntLevel > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Hunt level become 0 : Remove OnOpenDoorColliderOverlapBegin"))
            DoorDetectCollider->OnComponentBeginOverlap.Clear();
        }

        HuntLevel = Level;
    }
    
}

void APGBlindCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APGBlindCharacter, HuntLevel);
}

// Open(Break) all doors around character.
// When hunt level become 1 or 2 from 0, this function called to break doors.
void APGBlindCharacter::ForceOpenDoorsAroundCharacter()
{
    TArray<AActor*> OverlappedActors;
    DoorDetectCollider->GetOverlappingActors(OverlappedActors);
    for (AActor* OverlappedActor : OverlappedActors)
    {
        APGDoor1* OverlappedDoor = Cast<APGDoor1>(OverlappedActor);
        if (OverlappedDoor)
        {
            UE_LOG(LogTemp, Log, TEXT("Door around BlindCharacter was detected"));
            OverlappedDoor->TEST_OpenDoorByAI();
        }
    }
}

void APGBlindCharacter::BeginPlay()
{
    Super::BeginPlay();

    //collider 붙이기.
    if (BiteCollider && GetMesh())
    {
        BiteCollider->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Jaw1Socket"));
    }


    //callback 함수  등록
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
    
    // 플레이어만 감지 (필요 시 다른 조건 추가)
    // 현재는 tag 기반으로 작동. 로그 대신 다른 기능 추가하면 됨.
    if (OtherActor->ActorHasTag(FName("Player")))
    {
        UE_LOG(LogTemp, Log, TEXT(" Bite Successful"));
    }
    
    

}

void APGBlindCharacter::OnOpenDoorColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // if other actor is door, break the door!
    UE_LOG(LogTemp, Log, TEXT("OtherActor was detected by BlindCharacter"));

    APGDoor1* OverlappedDoor = Cast<APGDoor1>(OtherActor);
    if (OverlappedDoor)
    {
        UE_LOG(LogTemp, Log, TEXT("Door was detected by BlindCharacter"));
        OverlappedDoor->TEST_OpenDoorByAI();
    }
}
