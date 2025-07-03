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

	/** ��� Actor ���� */
	void SetTargetActor(AActor* NewTarget);

	/** ���� ���� ��� ��ȯ */
	AActor* GetTargetActor() const { return TargetToOrbit; }

	/** Ŭ���̾�Ʈ �Է¿� ���� �˵� Yaw�� ������Ʈ�ϴ� �Լ� */
	void UpdateOrbitYawInput(float DeltaYaw); // <--- �� �Լ��� �߰��մϴ�.

	// SpectatorPawn�� ����� ����ٴϰ� ȸ���ϴ� ������ ����ȭ�� �Լ� (�������� ȣ��)
	void UpdateSpectatorPositionAndRotation();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override {}
	virtual void AddControllerYawInput(float) override {}
	virtual void AddControllerPitchInput(float) override {}
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// ���� ��� Actor (�������� �����ϰ� Ŭ���̾�Ʈ�� ����)
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TargetToOrbit) // <-- ReplicatedUsing �߰�
	AActor* TargetToOrbit;

	UFUNCTION()
	void OnRep_TargetToOrbit();

	// ACharacter�� OnCharacterMovementUpdated ��������Ʈ�� ���ε��� �Լ�
	//UFUNCTION()
	//void OnTargetCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	float RotationSpeed = 90.0f; // �ʴ� 90�� ȸ��

	// ������κ����� ���� ������� �Ÿ�
	float CurrentOrbitDistance;

	// ��� ������ �����ϴ� ���� Yaw ����
	float CurrentOrbitYawAngle;
};
