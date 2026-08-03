// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Type/PGDifficultyTypes.h"
#include "Type/PGSessionTypes.h"

#include "PGMainMenuWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UPGSessionSlotWidget;
class UPGConfirmWidget;
class UPGSessionStatusWidget;
class UPGSettingMenuWidget;
class UWidgetSwitcher;
class UPGMainMenuProfileWidget;
class UPGDifficultySelectWidget;
class UPGMicSettingWidget;

class FOnlineSessionSearchResult;
class UPGAdvancedFriendsGameInstance;

/* TEST */
class UPGFinalScoreBoardWidget;
/* TEST */

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void AddSessionSlot(const FOnlineSessionSearchResult& SearchResult, int32 Index);

	UFUNCTION()
	void ClearSessionList();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SessionListContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SinglePlayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGSessionSlotWidget> SessionSlotWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGConfirmWidget> ConfirmWidgetClass;

	// Host
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGDifficultySelectWidget> DifficultySelectWidgetClass;

	// Single
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGDifficultySelectWidget> SinglePlaySelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGSessionStatusWidget> SessionStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGMicSettingWidget> MicSettingWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGConfirmWidget> ConfirmWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPGDifficultySelectWidget> DifficultySelectWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPGDifficultySelectWidget> SinglePlaySelectWidgetInstance;

	UPROPERTY()
	TObjectPtr<UPGSessionStatusWidget> SessionStatusWidgetInstance;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGSettingMenuWidget> SettingMenuWidgetInstance;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionMenuCanvas_BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SessionListCanvas_BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGMainMenuProfileWidget> PlayerProfileWidget;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnSinglePlayButtonClicked();

	UFUNCTION()
	void OnHostButtonClicked();

	void ShowSessionCreateWidget(const TSubclassOf<UPGDifficultySelectWidget>& WidgetClass, TObjectPtr<UPGDifficultySelectWidget>& InstanceRef);

	UFUNCTION()
	void OnHostSessionConfirmed(const FPGHostSessionOptions& Options);

	UFUNCTION()
	void StartHostSession();

	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void QuitGame();

	UFUNCTION()
	void OnOptionButtonClicked();

	UFUNCTION()
	void OnRefreshButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void HandleHostSessionStarted();

	UFUNCTION()
	void HandleHostSessionFinished(bool bWasSuccessful, const FText& ErrorMessage);

	void OnSessionsFound(const TArray<FOnlineSessionSearchResult>& SessionResults);

	UFUNCTION()
	void HandleFindSessionStarted();

	UFUNCTION()
	void HandleFindSessionFinished(bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinSessionStarted();

	UFUNCTION()
	void HandleJoinSessionFinished(bool bWasSuccessful, const FText& ErrorMessage);

	void CheckPendingNetworkFailure();
	void HandleNetworkFailure(const FText& ErrorMessage);

	void SetMainMenuButtonEnabled(bool bEnabled);
	void SetSessionListButtonEnabled(bool bEnabled);

	void ShowSessionStatusWidget(const FText& Message, bool bShowCloseButton = true);

	UFUNCTION()
	void HideSessionStatusWidget(float Delay = 0.0f);

	FTimerHandle SessionStatusWidgetTimerHandle;

private:
	TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
	FPGHostSessionOptions PendingHostOptions;
};
