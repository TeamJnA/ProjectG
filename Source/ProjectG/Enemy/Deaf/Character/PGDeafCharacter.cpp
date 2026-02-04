// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Deaf/Character/PGDeafCharacter.h"
#include "Enemy/Deaf/Ability/PGDeafAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Components/BoxComponent.h"

APGDeafCharacter::APGDeafCharacter()
{
    DeafAttributeSet = CreateDefaultSubobject<UPGDeafAttributeSet>("DeafAttributeSet");
}

float APGDeafCharacter::GetExplorationRadius() const
{
    return ExplorationRadius;
}

float APGDeafCharacter::GetExplorationWaitTime() const
{
    return ExplorationWaitTime;
}

EDeafHuntLevel APGDeafCharacter::GetHuntLevel() const
{
    return huntLevel;
}

void APGDeafCharacter::SetHuntLevel(EDeafHuntLevel newHuntLevel)
{
    checkf(HasAuthority(), TEXT("서버에서만 호출되어야 합니다."));

    //Break 할 수 있는 여부를 여기서 하드코딩으로 구현한 게 상당히 아쉽다. 시간 날때 구조를 고민해봐야 할 듯..
    //현재는 open은 항상 되고, break 여부만 달라지는데, 만약 후에 open도 못하는 경우가 생긴다면?
    //TODO : delegate로 수정하기.
    const bool canDoorBreakBefore = ((huntLevel == EDeafHuntLevel::Chase));
    const bool canDoorBreakAfter = ((newHuntLevel == EDeafHuntLevel::Chase));
    /*
    if (canDoorBreakBefore != canDoorBreakAfter)
    {//TODO : 임시로 두 상황 똑같이 문을 열도록 되어있음. 부수는 함수 추가되면 로직 수정 필요 + log 내부도.
        if (canDoorBreakAfter)
        {
            UE_LOG(LogTemp, Log, TEXT("%s : Add OnOpenDoorColliderOverlapBegin"), *UEnum::GetValueAsString(newHuntLevel));
            DoorDetectCollider->OnComponentBeginOverlap.Clear();
            DoorDetectCollider->OnComponentBeginOverlap.AddDynamic(this, &APGEnemyCharacterBase::OnOpenDoorColliderOverlapBegin);
            //TODO : BEGINPLAY에 delegate 추가
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("%s : Add OnOpenDoorColliderOverlapBegin"), *UEnum::GetValueAsString(newHuntLevel));
            DoorDetectCollider->OnComponentBeginOverlap.Clear();
            DoorDetectCollider->OnComponentBeginOverlap.AddDynamic(this, &APGEnemyCharacterBase::OnOpenDoorColliderOverlapBegin);
        }
    }
    huntLevel = newHuntLevel;
    */
}


void APGDeafCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APGDeafCharacter, huntLevel);
}

void APGDeafCharacter::BeginPlay()
{
    Super::BeginPlay();
}

