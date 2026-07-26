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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> NormalButton;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> HardButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostGameButton;

	bool bIsDifNormal;

	UFUNCTION()
	void OnNormalClicked();

	UFUNCTION()
	void OnHardClicked();

	UFUNCTION()
	void OnCancelClicked();

	UFUNCTION()
	void OnHostGameClicked();

private:
	TWeakObjectPtr<UUserWidget> ReturnFocusWidget;
};
