// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PGPlayerController.generated.h"

class APGSpectatorPawn;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ACharacter;

class APGDoor1;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APGPlayerController();

	void NotifyTravelFailed();

protected:	
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void Client_PostSeamlessTravel();

	UFUNCTION(Server, Reliable)
	void Server_ReportTravelFailed();	

	void OnSpectate(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_EnterSpectatorMode();

	void OnOrbitYaw(const FInputActionValue& Value);

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectateAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> OrbitYawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectateNextAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectatePrevAction;

	/** ���� SpectatorPawn ĳ�� */
	UPROPERTY(Transient) // �� ������ PlayerController�� Pawn�� Possess�ϸ� �ڿ������� Ŭ���̾�Ʈ�� ����ȭ�˴ϴ�.
	TObjectPtr<APGSpectatorPawn> ControlledSpectator;

	// *** �߿�: SpectateTargetCharacter�� �����ϵ��� ���� ***
	// Ŭ���̾�Ʈ�� PlayerController�� ���� ���� ������� �˾ƾ� �մϴ�.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_SpectateTargetCharacter) // <-- ReplicatedUsing �߰�
	TObjectPtr<ACharacter> SpectateTargetCharacter;

	UFUNCTION()
	void OnRep_SpectateTargetCharacter();

	// ���� ��� ���� ��, �÷��̾ Possess�ߴ� ���� ĳ���͸� ����
	UPROPERTY(Replicated) // �������� Ŭ���̾�Ʈ�� �����Ǿ�� �մϴ�.
	TObjectPtr<ACharacter> OriginalPlayerCharacter;
	// �÷��� ������ ĳ���͵��� ĳ���Ͽ� �����ϱ� ���� ����

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACharacter>> CachedAllPlayableCharacters; // TObjectPtr ��� ����

	void OnSpectateNext(const FInputActionValue& Value);
	void OnSpectatePrev(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_ChangeSpectateTarget(bool bNext);

	bool IsSpectateTargetCached = false;
};
