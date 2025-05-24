// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "PGLobbyPlayerController.generated.h"

class UPGLobbyWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APGLobbyPlayerController();

	UPGLobbyWidget* GetLobbyWidget() const;

	void SetReady();
	UFUNCTION(Client, Reliable)
	void Client_StartTravelCheckTimer();

protected:
	virtual void BeginPlay() override;
	void TravelCheck();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPGLobbyWidget> LobbyWidgetClass;

private:
	UPGLobbyWidget* LobbyWidgetInstance;
	FTimerHandle TravelCheckTimerHandle;
	
};
