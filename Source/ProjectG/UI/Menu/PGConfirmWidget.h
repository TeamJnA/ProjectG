// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGConfirmWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmClickedDelegate);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGConfirmWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetConfirmText(const FText& Message);

	FOnConfirmClickedDelegate OnConfirmClicked;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* YesButton;

	UPROPERTY(meta = (BindWidget))
	UButton* NoButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConfirmText;

	UFUNCTION()
	void OnYesButtonClicked();

	UFUNCTION()
	void OnNoButtonClicked();
};
