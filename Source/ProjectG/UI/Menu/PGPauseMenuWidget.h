// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPauseMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UPGConfirmWidget;
class UPGSettingMenuWidget;
class UPGFriendListWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnInviteFriendButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void ReturnToMainMenu();

	UFUNCTION()
	void OnDesktopButtonClicked();

	UFUNCTION()
	void ReturnToDesktop();

	UFUNCTION()
	void OnBackButtonClicked();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGConfirmWidget> ConfirmWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGConfirmWidget> ConfirmWidgetInstance;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InviteFriendButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGFriendListWidget> FriendListWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> DesktopButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton_OptionMenuCanvas;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton_InviteMenuCanvas;
};
