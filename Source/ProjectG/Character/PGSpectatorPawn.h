// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "PGSpectatorPawn.generated.h"

class ACharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	APGSpectatorPawn();

	/** 대상 Actor 설정 */
	void SetTargetActor(AActor* NewTarget);

	/** 현재 관전 대상 반환 */
	AActor* GetTargetActor() const { return TargetToOrbit; }

	/** 클라이언트 입력에 따라 궤도 Yaw를 업데이트하는 함수 */
	void UpdateOrbitYawInput(float DeltaYaw); // <--- 이 함수를 추가합니다.

	// SpectatorPawn이 대상을 따라다니고 회전하는 로직을 공통화한 함수 (서버에서 호출)
	void UpdateSpectatorPositionAndRotation();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override {}
	virtual void AddControllerYawInput(float) override {}
	virtual void AddControllerPitchInput(float) override {}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 관전 대상 Actor (서버에서 설정하고 클라이언트로 복제)
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TargetToOrbit) // <-- ReplicatedUsing 추가
	AActor* TargetToOrbit;

	UFUNCTION()
	void OnRep_TargetToOrbit();

	// ACharacter의 OnCharacterMovementUpdated 델리게이트에 바인딩될 함수
	//UFUNCTION()
	//void OnTargetCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	float RotationSpeed = 90.0f; // 초당 90도 회전

	// 대상으로부터의 현재 상대적인 거리
	float CurrentOrbitDistance;

	// 대상 주위를 공전하는 현재 Yaw 각도
	float CurrentOrbitYawAngle;
};
