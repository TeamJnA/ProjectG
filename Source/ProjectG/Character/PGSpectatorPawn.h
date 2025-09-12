// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "PGSpectatorPawn.generated.h"

class UInputAction;
struct FInputActionValue;
class APGPlayerCharacter;

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
	void SetSpectateTarget(AActor* NewTarget);

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
	AActor* TargetToOrbit = nullptr;

	UFUNCTION()
	void OnRep_TargetToOrbit();

	// �ʴ� ȸ��
	float RotationSpeed = 90.0f;
	// ������κ����� �Ÿ�
	float CurrentOrbitDistance = 150.0f;
	// ��� ������ �����ϴ� ���� Yaw ����
	float CurrentOrbitYawAngle = 0.0f;
};
