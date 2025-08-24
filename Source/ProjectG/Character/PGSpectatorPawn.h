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

	/** ��� Actor ���� */
	void SetTargetActor(AActor* NewTarget);

	/** ���� ���� ��� ��ȯ */
	AActor* GetTargetActor() const { return TargetToOrbit; }

	// SpectatorPawn�� ����� ����ٴϰ� ȸ���ϴ� ������ ����ȭ�� �Լ� (�������� ȣ��)
	void UpdateSpectatorPositionAndRotation();

	UFUNCTION(Server, Reliable)
	void Server_SetSpectateTarget(bool bNext);

	// Cached all PGPlayerCharaters to change spectate. 
	// If there's no characters to spectate, return false.
	bool InitCachedAllPlayableCharacters(const APGPlayerCharacter* PrevPGCharacter);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OrbitYawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectateNextAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

	void OnSpectateNext(const FInputActionValue& Value);
	void OnSpectatePrev(const FInputActionValue& Value);

	/** Ŭ���̾�Ʈ �Է¿� ���� �˵� Yaw�� ������Ʈ�ϴ� �Լ� */
	void OnOrbitYaw(const FInputActionValue& Value);

	bool IsSpectateTargetCached = false;

	UPROPERTY(Transient)
	TObjectPtr<APGPlayerCharacter> SpectateTargetCharacter;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APGPlayerCharacter>> CachedAllPlayableCharacters;

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
