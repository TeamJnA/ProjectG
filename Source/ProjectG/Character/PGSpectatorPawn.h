// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "PGSpectatorPawn.generated.h"

class UInputAction;
struct FInputActionValue;
class APGPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpectateTargetChangedDelegate, const APlayerState*, NewTarget);

/**
 * 
 */
UCLASS()
class PROJECTG_API APGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	APGSpectatorPawn();

	// 관전 대상 트래킹, 회전
	void UpdateSpectatorPositionAndRotation();	
	// 관전 대상 지정
	void SetSpectateTarget(const AActor* NewTarget, const APlayerState* NewTargetPlayerState);
	const AActor* GetSpectateTarget() const { return TargetToOrbit; }
	const APlayerState* GetSpectateTargetPlayerState() const { return TargetPlayerState; }

	UPROPERTY()
	FOnSpectateTargetChangedDelegate OnSpectateTargetChanged;

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override {}
	virtual void AddControllerYawInput(float) override {}
	virtual void AddControllerPitchInput(float) override {}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OrbitYawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

	// 클라이언트 입력에 따라 궤도 Yaw 업데이트
	void OnOrbitYaw(const FInputActionValue& Value);

private:
	// 관전 대상 Actor (서버에서 설정하고 클라이언트로 복제)
	UPROPERTY(ReplicatedUsing = OnRep_TargetToOrbit)
	TObjectPtr<AActor> TargetToOrbit = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_TargetPlayerState)
	TObjectPtr<APlayerState> TargetPlayerState = nullptr;

	UFUNCTION()
	void OnRep_TargetToOrbit();

	UFUNCTION()
	void OnRep_TargetPlayerState();

	// 초당 회전
	float RotationSpeed = 90.0f;
	// 대상으로부터의 거리
	float CurrentOrbitDistance = 150.0f;
	// 대상 주위를 공전하는 현재 Yaw 각도
	float CurrentOrbitYawAngle = 0.0f;
	float CurrentOrbitPitchAngle = 15.0f;

	bool bCanOrbit = true;
};
