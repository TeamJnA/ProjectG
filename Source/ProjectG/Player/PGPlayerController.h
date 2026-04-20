// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Net/VoiceConfig.h"
#include "Type/PGGameTypes.h"
#include "Game/PGGameMode.h"

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

	UFUNCTION(Client, Reliable)
	void Client_ShowLoadingScreen();

	UFUNCTION(Client, Reliable)
	void Client_HideLoadingScreen();

	void StartSpectate();
	void InitFinalScoreBoardWidget();

	void NotifyReadyToReturnLobby();

	UFUNCTION(Client, Reliable)
	void Client_ForceReturnToLobby();

	UFUNCTION(Client, Reliable)
	void Client_StartEscapeSequence(const EExitPointType ExitPoint, const bool bNeedAutomove = false, const FVector AutomoveLocation = FVector::ZeroVector);

	void SetSpectateEscapeCamera(EExitPointType ExitPointType);
	void SetSpectateNewTarget(const AActor* NewTarget, const APlayerState* NewTargetPlayerState);

	const APlayerState* GetCurrentSpectateTargetPlayerState() const;

	UFUNCTION(Client, Reliable)
	void Client_OnRevive();

	virtual void OnRep_Pawn() override;

	UFUNCTION(Client, Reliable)
	void Client_DisplayJumpscare(UTexture2D* JumpscareTexture); 

	void SetupPlayerForGameplay();

	// Check player was spawned with game start or revive.
	int32 bGameStartFirstSpawned : 1;

protected:	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void PostSeamlessTravel() override;

	void InitLocalVoice();

	void ReplaceInputMappingContext(const APawn* PawnType);

	UFUNCTION(Client, Reliable)
	void Client_PostSeamlessTravel();

	UFUNCTION(Server, Reliable)
	void Server_ReportTravelSuccess();

	UFUNCTION(Server, Reliable)
	void Server_SetReadyToReturnLobby();

	UFUNCTION()
	void OnEscapeMovementFinished();

	UFUNCTION(Server, Reliable)
	void Server_RequestFinishEscape();

	UFUNCTION(Server, Reliable)
	void Server_EnterSpectatorMode();

	UFUNCTION(Server, Reliable)
	void Server_ChangeSpectateTarget(bool bNext);

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> SpectateTargetList;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultGameplayMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> SpectateMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectateNextAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PushToTalkAction;

	int32 CurrentSpectateIndex = -1;

	void OnSpectateNext(const FInputActionValue& Value);
	void OnSpectatePrev(const FInputActionValue& Value);
	void OnShowPauseMenu(const FInputActionValue& Value);

	void OnPushToTalkToggled(const FInputActionValue& Value);

public:
	void ApplyVoiceMode();
	void HandlePushToTalkToggle();
	bool IsPushToTalkReady() const;
	FORCEINLINE bool IsPushToTalkActive() const { return bPushToTalkActive; }
	void SetPushToTalkPrimed() { bPushToTalkPrimed = true; }
	float GetPushToTalkElapsed() const;

	///
	///		Audio comp. Sound parts
	///
public:
	void InitializeGameplayBGM();

	UFUNCTION(Client, Reliable)

	void Client_PlayGameplayBGM();

	void StopGameplayBGM();

protected:
	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> GameplayBGMSound;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> GameplayBGMPlayer;

public:
	void RefreshVoiceChannel();

	UFUNCTION(Client, Reliable)
	void Client_StopVoiceAndCleanup(ECleanupActionType ActionType, const FUniqueNetIdRepl& TargetNetId);

	UFUNCTION(Client, Reliable)
	void Client_ExecuteSoloAction(ECleanupActionType ActionType);

	UFUNCTION(Server, Reliable)
	void Server_RequestSoloLeave(ECleanupActionType ActionType);

	UFUNCTION(Server, Reliable)
	void Server_RequestSessionDestruction(bool bServerQuit);

private:
	void PerformCleanup(const FUniqueNetIdRepl& TargetNetId);
	void PerformSessionEndAction(ECleanupActionType ActionType);

	UPROPERTY()
	TSet<FUniqueNetIdRepl> Leavers;

	float PushToTalkStartTime = 0.0f;

	bool bIsLeavingSession = false;

	bool bPushToTalkActive = false;
	bool bPushToTalkPrimed = false;
};
