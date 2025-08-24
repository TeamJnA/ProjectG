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
class UPGFinalScoreBoardWidget;
class UPGPauseMenuWidget;

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
	void ReplaceInputMappingContext(const APawn* PawnType);

	virtual void PostSeamlessTravel() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Client, Reliable)
	void Client_PostSeamlessTravel();

	UFUNCTION(Server, Reliable)
	void Server_ReportTravelFailed();	

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
	TObjectPtr<UInputAction> OrbitYawAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectateNextAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

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
	TObjectPtr<APGPlayerCharacter> OriginalPlayerCharacter;
	// �÷��� ������ ĳ���͵��� ĳ���Ͽ� �����ϱ� ���� ����

	UPROPERTY(Transient)
	TArray<TObjectPtr<ACharacter>> CachedAllPlayableCharacters;

	void OnSpectateNext(const FInputActionValue& Value);
	void OnSpectatePrev(const FInputActionValue& Value);

	void OnOrbitYaw(const FInputActionValue& Value);

	void OnShowPauseMenu(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_ChangeSpectateTarget(bool bNext);

	bool IsSpectateTargetCached = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPGFinalScoreBoardWidget> FinalScoreBoardWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGFinalScoreBoardWidget> FinalScoreBoardWidgetInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPGPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGPauseMenuWidget> PauseMenuWidgetInstance;
};
