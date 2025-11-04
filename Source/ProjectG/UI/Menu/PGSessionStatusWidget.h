// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGSessionStatusWidget.generated.h"

class UTextBlock;
class UButton;
class UThrobber;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGSessionStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetStatusMessage(const FText& Message, bool bShowCloseButton = true);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UThrobber> Throbber;

	UFUNCTION()
	void OnCloseButtonClicked();
};
