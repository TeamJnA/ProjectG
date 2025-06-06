// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGLobbyWidget.generated.h"

class UScrollBox;
class UButton;
class UTextBlock;
class UPGSessionSlotWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void AddSessionSlot(const FString& ServerName, int32 Index);

	UFUNCTION(BlueprintCallable)
	void ClearSessionList();

	UFUNCTION(BlueprintCallable)
	void OnReadyClicked();

	//UFUNCTION(BlueprintCallable)
	//void OnRefreshClicked();

	UPROPERTY(meta = (BindWidget))
	UScrollBox* SessionListContainer;

	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;

	UPROPERTY()
	APlayerController* CachedPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGSessionSlotWidget> SessionSlotWidgetClass;

protected:
	virtual void NativeConstruct() override;
};
