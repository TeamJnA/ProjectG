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

	/** 현재 SpectatorPawn 캐시 */
	UPROPERTY(Transient) // 이 변수는 PlayerController가 Pawn을 Possess하면 자연스럽게 클라이언트에 동기화됩니다.
	TObjectPtr<APGSpectatorPawn> ControlledSpectator;

	// *** 중요: SpectateTargetCharacter를 복제하도록 변경 ***
	// 클라이언트의 PlayerController가 누가 관전 대상인지 알아야 합니다.
	UPROPERTY(Transient, ReplicatedUsing = OnRep_SpectateTargetCharacter) // <-- ReplicatedUsing 추가
	TObjectPtr<ACharacter> SpectateTargetCharacter;

	UFUNCTION()
	void OnRep_SpectateTargetCharacter();

	// 관전 모드 진입 전, 플레이어가 Possess했던 원래 캐릭터를 저장
	UPROPERTY(Replicated) // 서버에서 클라이언트로 복제되어야 합니다.
	TObjectPtr<APGPlayerCharacter> OriginalPlayerCharacter;
	// 플레이 가능한 캐릭터들을 캐싱하여 재사용하기 위한 변수

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
