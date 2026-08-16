// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/PGHelperTypes.h"
#include "PGHelperExitEntryWidget.generated.h"

class UTextBlock;
class UHorizontalBox;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGHelperExitEntryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetEntry(int32 InSpeciesKey, const FPGHelperEntryRow& Row, const TMap<EPGExitItemType, int32>& UnlockedItemIds, bool bDepleted);
	void PlayIntroAnim();

	void UpdateInPlace(const TMap<EPGExitItemType, int32>& UnlockedCounts, bool bDepleted);

	bool IsIntroComplete() const { return bIntroComplete; }
	int32 GetSpeciesKey() const { return SpeciesKey; }

protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void TypewriterStep();

	UFUNCTION()
	void RequirementTypewriterStep();

	void RebuildRequirementString(const TMap<EPGExitItemType, int32>& UnlockedCounts);

	void CompleteIntro();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RequirementText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DepletedLine;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	FLinearColor UnlockedColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	FLinearColor LockedColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

	UPROPERTY()
	TArray<FPGHelperRequiredItem> CachedItems;

	FString FullDisplayName;
	FString FullRequirementText;

	FTimerHandle TypewriterTimerHandle;
	FTimerHandle RequirementTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	float TypewriterStepInterval = 0.03f;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	float RequirementStartDelay = 0.2f;

	int32 TypewriterIndex = 0;
	int32 RequirementIndex = 0;

	int32 SpeciesKey = 0;
	bool bIsDepleted = false;
	bool bIntroComplete = false;
};
