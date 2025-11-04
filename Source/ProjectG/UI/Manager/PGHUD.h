// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PGHUD.generated.h"

class UPGAttributesWidget;
class UPGInventoryWidget;
class UPGInventoryComponent;
class UPGMessageManagerWidget;
class UPGCrosshairWidget;
class UPGInteractionProgressWidget;
class UPGMainMenuWidget;
class UPGLobbyWidget;
class UPGSpectatorWidget;
class UPGScoreBoardWidget;
class UPGFinalScoreBoardWidget;
class UPGPauseMenuWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGHUD : public AHUD
{
	GENERATED_BODY()

public:
	void Init();
	void InitMainMenuWidget();
	void InitLobbyWidget();
	void InitSpectatorWidget();
	void InitScoreBoardWidget();
	void InitFinalScoreBoardWidget();
	void InitPauseMenuWidget();
	void UpdateInteractionProgress(float Progress);
	void DisplayInteractionFailedMessage(const FText& Message, float Duration);

	void ClearViewport();

	// MessageManagerWidget Getter
	UPGMessageManagerWidget* GetMessageManagerWidget() const { return MessageManagerWidget; }
	// InventoryWidgetGetter
	UPGInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }
	// ScoreBoardWidgetGetter
	UPGScoreBoardWidget* GetScoreBoardWidget() const { return ScoreBoardWidget; }

protected:
	APGHUD();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGMainMenuWidget> MainMenuWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGLobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGSpectatorWidget> SpectatorWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGAttributesWidget> AttributeWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGMessageManagerWidget> MessageManagerWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGScoreBoardWidget> ScoreBoardWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGFinalScoreBoardWidget> FinalScoreBoardWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInteractionProgressWidget> InteractionProgressWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPGMainMenuWidget> MainMenuWidget;

	UPROPERTY()
	TObjectPtr<UPGLobbyWidget> LobbyWidget;
	
	UPROPERTY()
	TObjectPtr<UPGSpectatorWidget> SpectatorWidget;

	UPROPERTY()
	TObjectPtr<UPGAttributesWidget> AttributeWidget;

	UPROPERTY()
	TObjectPtr<UPGInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UPGMessageManagerWidget> MessageManagerWidget;

	UPROPERTY()
	TObjectPtr<UPGScoreBoardWidget> ScoreBoardWidget;

	UPROPERTY()
	TObjectPtr<UPGFinalScoreBoardWidget> FinalScoreBoardWidget;

	UPROPERTY()
	TObjectPtr<UPGPauseMenuWidget> PauseMenuWidget;

	UPROPERTY()
	TObjectPtr<UPGCrosshairWidget> CrosshairWidget;

	UPROPERTY()
	TObjectPtr<UPGInteractionProgressWidget> InteractionProgressWidget;
};
