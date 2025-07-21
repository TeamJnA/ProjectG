// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "PGLobbyPlayerController.generated.h"

class UPGMainMenuWidget;
class UPGLobbyWidget;
class ACameraActor;

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
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_SpawnAndPossessPlayer();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
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

private:
	TObjectPtr<UPGMainMenuWidget> MainMenuWidgetInstance;

	TObjectPtr<UPGLobbyWidget> LobbyWidgetInstance;

};
