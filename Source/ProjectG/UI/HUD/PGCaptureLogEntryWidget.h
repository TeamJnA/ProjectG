// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/PGPhotoTypes.h"
#include "PGCaptureLogEntryWidget.generated.h"

class UTextBlock;
class UImage;
class UOverlay;
class UTextureRenderTarget2D;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGCaptureLogEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetThumbnail(UTextureRenderTarget2D* RT);
	void BeginLog(const TArray<FCaptureLogLine>& InLines);

	void ForceRemove();

protected:
	virtual void NativeDestruct() override;

	void StartThumbnailFadeIn();
	void OnThumbnailFadeStep();

	void TryStart();
	void StartLineIn();
	void StartTransition();
	UTextBlock* MakeLineText(const FCaptureLogLine& Line);
	void ApplyLineState(UTextBlock* Text, FVector2D Translation, float Opacity);
	void AdvanceTypewriter();
	void StartEntryFadeOut();
	void Driver();
	FORCEINLINE bool IsTypingDone() const { return CharIndex >= CurrentFull.Len(); }

	void ClearAllTimers();

	UPROPERTY(EditDefaultsOnly, Category = "Log")
	FSlateFontInfo LineFont;

	FString CurrentFull;

	UPROPERTY(EditDefaultsOnly, Category = "Log")
	FLinearColor ValidColor = FLinearColor(0.3f, 1.0f, 0.3f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Log")
	FLinearColor InvalidColor = FLinearColor(1.0f, 0.3f, 0.3f, 1.0f);

	UPROPERTY()
	TArray<FCaptureLogLine> PendingLines;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Thumbnail;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> LineSlot;

	UPROPERTY()
	TObjectPtr<UTextBlock> ActiveText;

	UPROPERTY()
	TObjectPtr<UTextBlock> OutgoingText;

	FTimerHandle FadeInHandle;
	FTimerHandle DriverHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Thumbnail")
	float ThumbnailFadeInDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float SlideInOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float SlideOutOffset = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float SlideDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float TypeInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float LineHoldDuration = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Log|Line")
	float FadeOutDuration = 0.3f;

	float PhaseElapsed = 0.0f;
	float ThumbnailAlpha = 0.0f;
	float TypeAccum = 0.0f;
	static constexpr float DriverStep = 0.02f;

	int32 LineIndex = 0;
	int32 CharIndex = 0;

	bool bThumbnailFaded = false;
	bool bHasLines = false;
	bool bStarted = false;

	enum class EPhase : uint8 { Idle, LineIn, Hold, Transition, EntryFadeOut };
	EPhase Phase = EPhase::Idle;
};
