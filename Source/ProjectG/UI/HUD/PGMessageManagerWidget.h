// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGMessageManagerWidget.generated.h"

class APGPlayerCharacter;
class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMessageManagerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindMessageEntry(APGPlayerCharacter* PlayerCharacter);
	void ShowFailureIcon(UMaterialInterface* Icon, FVector2D IconSize, float Duration, bool bAffectInteractPrompt = true);
	
protected:
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleOnStareTargetUpdate(AActor* TargetActor);

	void SetInteractIcon(bool bShow);
	void SetInteractText(const FText& InText);

	void SetFailureIcon(UMaterialInterface* Icon, FVector2D IconSize);
	void OnFailureTimerExpired();
	void RestoreInteractText();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> InteractIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InteractText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FailureIconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FailureXImage;

	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<UTexture2D> InteractIconTexture;

	UPROPERTY(EditAnywhere, Category = "UI")
	FVector2D InteractIconSize = FVector2D(32.0f, 32.0f);

	UPROPERTY(EditAnywhere, Category = "UI")
	FText FailedText = FText::FromString(TEXT("Failed"));

	FTimerHandle FailureIconTimerHandle;

private:
	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor NormalTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor FailureTint = FLinearColor::Red;

	TWeakObjectPtr<APGPlayerCharacter> CharacterRef;
};
