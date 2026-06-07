// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGEnemyToastWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGEnemyToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void PlayToast(const FText& Text);
	void HideBrieflyForTransition();
	void ResetToast();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void PlayInternal(const FText& Text);
	void OnTypewriterDelay();
	void OnTypewriterStep();

	UFUNCTION()
	void OnToastAnimFinished();

	UFUNCTION()
	void RestoreAfterTransition();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EnemyMessageText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ToastAnim;

	TArray<FText> PendingTexts;

	FString FullString;
	FTimerHandle TypewriterDelayHandle;
	FTimerHandle TypewriterStepHandle;
	FTimerHandle TransitionRestoreHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Toast|Typewriter")
	float TypewriterStartDelay = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Toast|Typewriter")
	float TypewriterStepInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Toast")
	float TransitionHideDuration = 0.3f;
	
	int32 CharIndex = 0;

	bool bIsAnimating = false;
};
