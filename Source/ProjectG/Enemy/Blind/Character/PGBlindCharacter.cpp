// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/Ability/PGBlindAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"


APGBlindCharacter::APGBlindCharacter()
{
    BlindAttributeSet = CreateDefaultSubobject<UPGBlindAttributeSet>("BlindAttributeSet");

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

    static ConstructorHelpers::FObjectFinder<UAnimMontage> SniffMontageRef(TEXT("/Game/ProjectG/Enemy/Blind/Character/Animation/Other/AM_BlindRoar.AM_BlindSniff"));
    if (SniffMontageRef.Succeeded())
    {
        SniffMontage = SniffMontageRef.Object;
    }

}

void APGBlindCharacter::SetHuntLevel(EBlindHuntLevel newHuntLevel)
{
    checkf(HasAuthority(), TEXT("서버에서만 호출되어야 합니다."));

    //Break 할 수 있는 여부를 여기서 하드코딩으로 구현한 게 상당히 아쉽다. 시간 날때 구조를 고민해봐야 할 듯..
    //현재는 open은 항상 되고, break 여부만 달라지는데, 만약 후에 open도 못하는 경우가 생긴다면?
    const bool canDoorBreakBefore = ((HuntLevel == EBlindHuntLevel::Investigation) || (HuntLevel == EBlindHuntLevel::Chase));
    const bool canDoorBreakAfter = ((newHuntLevel == EBlindHuntLevel::Investigation) || (newHuntLevel == EBlindHuntLevel::Chase));

    if (canDoorBreakBefore != canDoorBreakAfter)
    {//TODO : 임시로 두 상황 똑같이 문을 열도록 되어있음. 부수는 함수 추가되면 로직 수정 필요 + log 내부도..
        if (canDoorBreakAfter)
        {
            UE_LOG(LogTemp, Log, TEXT("%s : Add OnOpenDoorColliderOverlapBegin"), *UEnum::GetValueAsString(newHuntLevel));
            DoorDetectCollider->OnComponentBeginOverlap.Clear();
            DoorDetectCollider->OnComponentBeginOverlap.AddDynamic(this, &APGEnemyCharacterBase::OnOpenDoorColliderOverlapBegin);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("%s : Add OnOpenDoorColliderOverlapBegin"), *UEnum::GetValueAsString(newHuntLevel));
            DoorDetectCollider->OnComponentBeginOverlap.Clear();
            DoorDetectCollider->OnComponentBeginOverlap.AddDynamic(this, &APGEnemyCharacterBase::OnOpenDoorColliderOverlapBegin);
        }
    }
    HuntLevel = newHuntLevel;
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
            OverlappedDoor->TEST_OpenDoorByAI(this);
        }
    }
}
void APGBlindCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void APGBlindCharacter::OnOpenDoorColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // if other actor is door, break the door!
    UE_LOG(LogEnemyCharacter, Log, TEXT("OtherActor was detected by BlindCharacter Door Collision"));

    APGDoor1* OverlappedDoor = Cast<APGDoor1>(OtherActor);
    if (OverlappedDoor)
    {
        UE_LOG(LogTemp, Log, TEXT("Door was detected by BlindCharacter"));
        OverlappedDoor->TEST_OpenDoorByAI(this);
    }
}
