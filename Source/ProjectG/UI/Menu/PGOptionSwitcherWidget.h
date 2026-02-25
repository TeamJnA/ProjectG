// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGOptionSwitcherWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOptionChangedDelegate, int32, OptionIndex);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGOptionSwitcherWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	/**
	 * Set the selected option by index.
	 * @param NewIndex - Index to select
	 * @param bDoCallback - If true, broadcast OnOptionChanged
	 */
	UFUNCTION(BlueprintCallable, Category = "Option")
	void SetSelectedIndex(int32 NewIndex, bool bDoCallback = false);

	/** Get current option index */
	UFUNCTION(BlueprintCallable, Category = "Option")
	int32 GetCurrentOptionIndex() const { return CurrentOption; }

	/** Fired when the option changes */
	UPROPERTY(BlueprintAssignable, Category = "Option")
	FOnOptionChangedDelegate OnOptionChanged;

protected:
	/** Available options - set per instance in Details panel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	TArray<FString> Options;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PrevOptionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextOptionButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OptionText;

	/** Current selected index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	int32 CurrentOption = 0;

	/** Max index the user can navigate to via buttons. -1 = no limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option")
	int32 MaxSelectableIndex = -1;

private:
	UFUNCTION()
	void OnPrevClicked();

	UFUNCTION()
	void OnNextClicked();

	void UpdateDisplay();
	void UpdateButtonStates();	
};
