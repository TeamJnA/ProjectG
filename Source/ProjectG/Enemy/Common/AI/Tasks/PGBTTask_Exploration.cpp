// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AI/Tasks/PGBTTask_Exploration.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Enemy/Common/AI/Controllers/PGEnemyAIControllerBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Game/PGGameState.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Charger/Character/PGChargerCharacter.h"


UPGBTTask_Exploration::UPGBTTask_Exploration(FObjectInitializer const& ObjectInitializer) :
	UBTTask_BlackboardBase{ ObjectInitializer }
{
	NodeName = "Exploration";
}

EBTNodeResult::Type UPGBTTask_Exploration::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APGEnemyAIControllerBase* const Cont = Cast<APGEnemyAIControllerBase>(OwnerComp.GetAIOwner());
    if (!Cont)
    {
        return EBTNodeResult::Failed;
    }

    APGEnemyCharacterBase* const Enemy = Cast<APGEnemyCharacterBase>(Cont->GetPawn());
    if (!Enemy || !Enemy->Implements<UPGAIExplorationInterface>())
    {
        return EBTNodeResult::Failed;
    }

    IPGAIExplorationInterface* IExploration = Cast<IPGAIExplorationInterface>(Enemy);
    if (!IExploration)
    {
        return EBTNodeResult::Failed;
    }

    float ExplorationWaitTime = IExploration->GetExplorationWaitTime();
    OwnerComp.GetBlackboardComponent()->SetValueAsFloat("WaitTime", ExplorationWaitTime);

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        return EBTNodeResult::Failed;
    }

    const FVector CurrentLocation = Enemy->GetActorLocation();

    // Waypoint 기반 탐색
    if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
    {
        FVector WaypointTarget = GS->GetExplorationTarget(CurrentLocation);
        if (!WaypointTarget.IsZero())
        {
            FNavLocation NavLoc;
            if (NavSys->GetRandomPointInNavigableRadius(WaypointTarget, 1000.0f, NavLoc))
            {
#if WITH_EDITOR
                // 선: AI → 선택된 Waypoint까지 경로, 구체: 선택된 Waypoint
                FColor DebugColor;
                if (Enemy->IsA(APGBlindCharacter::StaticClass()))
                    DebugColor = FColor::Red;
                else if (Enemy->IsA(APGChargerCharacter::StaticClass()))
                    DebugColor = FColor::Yellow;
                else
                    DebugColor = FColor::Green;

                DrawDebugLine(GetWorld(), CurrentLocation, NavLoc.Location, DebugColor, false, 5.0f, 0, 3.0f);
                DrawDebugSphere(GetWorld(), NavLoc.Location, 50.0f, 8, DebugColor, false, 5.0f);
                DrawDebugString(GetWorld(), NavLoc.Location + FVector(0, 0, 70), FString::Printf(TEXT("%s → WP"), *Enemy->GetName()), nullptr, DebugColor, 5.0f);
#endif

                OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), NavLoc.Location);
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return EBTNodeResult::Succeeded;
            }
        }
    }

    // Waypoint 탐색 실패 시 기존 랜덤 탐색 폴백
    float ExplorationRadius = IExploration->GetExplorationRadius();
    FNavLocation Loc;
    if (NavSys->GetRandomPointInNavigableRadius(CurrentLocation, ExplorationRadius, Loc))
    {
#if !UE_BUILD_SHIPPING
        DrawDebugLine(GetWorld(), CurrentLocation, Loc.Location, FColor::White, false, 5.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), Loc.Location, 50.0f, 8, FColor::White, false, 5.0f);
        DrawDebugString(GetWorld(), Loc.Location + FVector(0, 0, 70), TEXT("Fallback"), nullptr, FColor::White, 5.0f);
#endif
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Loc.Location);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return EBTNodeResult::Succeeded;
    }

	return EBTNodeResult::Failed;
}
