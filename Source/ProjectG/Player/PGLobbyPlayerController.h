// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Type/PGGameTypes.h"

#include "PGLobbyPlayerController.generated.h"

class ACameraActor;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APGLobbyPlayerController();

	UFUNCTION(Client, Reliable)
	void Client_ForceReturnToLobby();

	UFUNCTION(Client, Reliable)
	void Client_ShowLoadingScreen();

protected:
	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnRep_Pawn() override;

	void InitLocalVoice();
	void HideLoadingScreenDelayed();

	UFUNCTION(Server, Reliable)
	void Server_SpawnAndPossessPlayer();

	void SetupLobbyUI();
	void SetupMainMenuUI();
	void SetupMainMenuView();
	void OnShowPauseMenu(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ACameraActor> LobbyCameraClass;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

public:
	void RefreshVoiceChannel();

	UFUNCTION(Server, Reliable)
	void Server_RequestSoloLeave(ECleanupActionType ActionType);

	UFUNCTION(Server, Reliable)
	void Server_RequestSessionDestruction(bool bServerQuit);
	
	UFUNCTION(Client, Reliable)
	void Client_StopVoiceAndCleanup(ECleanupActionType ActionType, const FUniqueNetIdRepl& TargetNetId);

	UFUNCTION(Client, Reliable)
	void Client_ExecuteSoloAction(ECleanupActionType ActionType);

private:
	void PerformCleanup(const FUniqueNetIdRepl& TargetNetId);
	void PerformSessionEndAction(ECleanupActionType ActionType);

	UPROPERTY()
	TSet<FUniqueNetIdRepl> Leavers;

	bool bIsLeavingSession = false;
};
