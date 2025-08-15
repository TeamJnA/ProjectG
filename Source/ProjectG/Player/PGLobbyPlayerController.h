// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "PGLobbyPlayerController.generated.h"

class ACameraActor;

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UPGMainMenuWidget;
class UPGLobbyWidget;
class UPGPauseMenuWidget;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APGLobbyPlayerController();

	UPGMainMenuWidget* GetMainMenuWidget() const;

	void SetReady();

	UFUNCTION(Client, Reliable)
	void Client_ForceReturnToLobby();
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void Server_SpawnAndPossessPlayer();

	void SetupLobbyUI();
	void SetupMainMenuUI();
	void SetupMainMenuView();

	IOnlineSessionPtr SessionInterface;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPGMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGLobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TSubclassOf<ACharacter> LobbyCharacterClass;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ACameraActor> LobbyCameraClass;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShowPauseMenuAction;

	void OnShowPauseMenu(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPGPauseMenuWidget> PauseMenuWidgetClass;

private:
	TObjectPtr<UPGMainMenuWidget> MainMenuWidgetInstance;

	TObjectPtr<UPGLobbyWidget> LobbyWidgetInstance;

	TObjectPtr<UPGPauseMenuWidget> PauseMenuWidgetInstance;

};
