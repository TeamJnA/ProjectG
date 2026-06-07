// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGExitToastWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGExitToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowToast();
	void DismissToast();
	void HideImmediate();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void StartTypewriter();
	void OnTypewriterDelay();
	void OnTypewriterStep();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ExitMessageText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Toast")
	FText ExitToastText;

	UPROPERTY(EditDefaultsOnly, Category = "Toast|Typewriter")
	float TypewriterStartDelay = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Toast|Typewriter")
	float TypewriterStepInterval = 0.03f;

	FString FullString;
	int32 CharIndex = 0;
	FTimerHandle TypewriterDelayHandle;
	FTimerHandle TypewriterStepHandle;
};
