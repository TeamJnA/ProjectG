// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/PGDifficultyTypes.h"
#include "PGDifficultySelectWidget.generated.h"

class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDifficultySelectedDelegate, EPGDifficulty, SelectedDifficulty);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGDifficultySelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnDifficultySelectedDelegate OnDifficultySelected;
	void SetReturnFocusWidget(UUserWidget* InWidget) { ReturnFocusWidget = InWidget; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NormalButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HardButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UFUNCTION()
	void OnNormalClicked();

	UFUNCTION()
	void OnHardClicked();

	UFUNCTION()
	void OnCancelClicked();

private:
	TWeakObjectPtr<UUserWidget> ReturnFocusWidget;
};
