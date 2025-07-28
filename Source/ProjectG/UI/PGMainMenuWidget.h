// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGMainMenuWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UPGSessionSlotWidget;
class UPGConfirmWidget;
class UPGSettingMenuWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AddSessionSlot(const FString& ServerName, int32 Index);

	UFUNCTION(BlueprintCallable)
	void ClearSessionList();

	//UFUNCTION(BlueprintCallable)
	//void OnRefreshClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SessionListContainer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RefreshButton;

	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGSessionSlotWidget> SessionSlotWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGConfirmWidget> ConfirmWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGConfirmWidget> ConfirmWidgetInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGSettingMenuWidget> SettingMenuWidgetClass;

	UPROPERTY()
	TObjectPtr<UPGSettingMenuWidget> SettingMenuWidgetInstance;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();
};
