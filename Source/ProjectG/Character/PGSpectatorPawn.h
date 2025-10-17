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

	// ���� ��� Ʈ��ŷ, ȸ��
	void UpdateSpectatorPositionAndRotation();	
	// ���� ��� ����
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

	// Ŭ���̾�Ʈ �Է¿� ���� �˵� Yaw ������Ʈ
	void OnOrbitYaw(const FInputActionValue& Value);

private:
	// ���� ��� Actor (�������� �����ϰ� Ŭ���̾�Ʈ�� ����)
	UPROPERTY(ReplicatedUsing = OnRep_TargetToOrbit)
	TObjectPtr<AActor> TargetToOrbit = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_TargetPlayerState)
	TObjectPtr<APlayerState> TargetPlayerState = nullptr;

	UFUNCTION()
	void OnRep_TargetToOrbit();

	UFUNCTION()
	void OnRep_TargetPlayerState();

	// �ʴ� ȸ��
	float RotationSpeed = 90.0f;
	// ������κ����� �Ÿ�
	float CurrentOrbitDistance = 150.0f;
	// ��� ������ �����ϴ� ���� Yaw ����
	float CurrentOrbitYawAngle = 0.0f;
	float CurrentOrbitPitchAngle = 15.0f;

	bool bCanOrbit = true;
};
