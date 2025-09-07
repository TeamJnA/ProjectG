// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "Blueprint/UserWidget.h"

#include "PGPlayerController.generated.h"

class APGSpectatorPawn;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ACharacter;
class APGPlayerCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APGPlayerController();

	void StartSpectate();
	void InitFinalScoreBoardWidget();

	void NotifyReadyToReturnLobby();

	UFUNCTION(Client, Reliable)
	void Client_ForceReturnToLobby();

	virtual void OnRep_Pawn() override;

protected:	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void PostSeamlessTravel() override;
	void ReplaceInputMappingContext(const APawn* PawnType);

	UFUNCTION(Client, Reliable)
	void Client_PostSeamlessTravel();

	UFUNCTION(Server, Reliable)
	void Server_ReportTravelSuccess();

	UFUNCTION(Server, Reliable)
	void Server_SetReadyToReturnLobby();

	UFUNCTION(Server, Reliable)
	void Server_EnterSpectatorMode();

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultGameplayMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> SpectateMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

	/** ���� SpectatorPawn ĳ�� */
	UPROPERTY(Transient) // �� ������ PlayerController�� Pawn�� Possess�ϸ� �ڿ������� Ŭ���̾�Ʈ�� ����ȭ�˴ϴ�.
	TObjectPtr<APGSpectatorPawn> ControlledSpectator;

	void OnShowPauseMenu(const FInputActionValue& Value);
};
