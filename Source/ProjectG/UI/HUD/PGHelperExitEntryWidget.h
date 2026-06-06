// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGHelperExitEntryWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UImage;
struct FPGHelperEntryRow;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGHelperExitEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(int32 InSpeciesKey, const FPGHelperEntryRow& Row, const TSet<FName>& UnlockedItemIds, bool bDepleted);
	void PlayIntroAnim();

	void UpdateInPlace(const TSet<FName>& UnlockedItemIds, bool bDepleted);

	bool IsIntroComplete() const { return bIntroComplete; }
	int32 GetSpeciesKey() const { return SpeciesKey; }

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void TypewriterStep();

	UFUNCTION()
	void IconAppearStep();

	void CompleteIntro();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ItemIconBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DepletedLine;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	FLinearColor UnlockedColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	FLinearColor LockedColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

	FString FullDisplayName;

	UPROPERTY()
	TArray<TObjectPtr<UImage>> SpawnedIconImages;

	TArray<FName> SpawnedIconItemIds;

	FTimerHandle TypewriterTimerHandle;
	FTimerHandle IconAppearTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	float TypewriterStepInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	float IconStartDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	float IconStepInterval = 0.1f;

	int32 TypewriterIndex = 0;
	int32 IconAppearIndex = 0;

	int32 SpeciesKey = 0;
	bool bIsDepleted = false;
	bool bIntroComplete = false;
};
