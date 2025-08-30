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

public:
	void Init(APlayerController* PC);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnResumeButtonClicked();

	UFUNCTION()
	void OnInviteFriendButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();

	UFUNCTION()
	void OnMainMenuButtonClicked();

	UFUNCTION()
	void OnDesktopButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

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

private:
	TObjectPtr<APlayerController> OwningPC;

};
