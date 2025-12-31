// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGFinalScoreBoardWidget.generated.h"

class UPGPlayerEntryWidget;
class APGPlayerCharacter;
class UVerticalBox;
class UButton;
class UPGConfirmWidget;
class UPGAdvancedFriendsGameInstance;
class APGGameState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGFinalScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindPlayerEntry();

	UFUNCTION()
	void UpdatePlayerEntry();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnReturnToMainMenuButtonClicked();

	UFUNCTION()
	void ReturnToMainMenu();

	UFUNCTION()
	void OnReturnToLobbyButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToMainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGConfirmWidget> ConfirmWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGConfirmWidget> ConfirmWidgetInstance;

private:
	TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
	TWeakObjectPtr<APGGameState> GSRef;
};
