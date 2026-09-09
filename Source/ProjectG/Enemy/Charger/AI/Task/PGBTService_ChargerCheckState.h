// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "PGBTService_ChargerCheckState.generated.h"

class APGChargerAIController;
class APGChargerCharacter;
class UBlackboardComponent;

struct FPGChargerCheckStateMemory
{
	FVector LastLocation = FVector::ZeroVector;
	float StuckTime = 0.0f;
	bool bHasLastLocation = false;
};

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGBTService_ChargerCheckState : public UBTService
{
	GENERATED_BODY()

public:
	UPGBTService_ChargerCheckState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

	/**
	* 시야 재탐색 후 블랙보드의 타겟 정보(TargetActor / TargetLocation / IsTargetVisible)와
	* Charger의 head look-at을 갱신
	* 아무도 보이지 않으면 TargetActor를 비우고 마지막 위치를 계속 응시
	* @return 갱신된 타겟 (없으면 nullptr)
	*/
	AActor* UpdateTargeting(APGChargerAIController* AIC, UBlackboardComponent* BB, APGChargerCharacter* Charger) const;

	UPROPERTY(EditAnywhere, Category = "Adjust")
	float AdjustProgressThreshold = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Adjust")
	float AdjustStuckTimeLimit = 1.5f;

	/** Adjusting 중 이 거리 안으로 들어오면 도착으로 간주 */
	UPROPERTY(EditAnywhere, Category = "Adjust")
	float ArrivalThreshold = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Stare")
	float BaseStareThreshold = 5.0f;
};
