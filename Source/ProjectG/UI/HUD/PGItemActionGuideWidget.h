// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGItemActionGuideWidget.generated.h"

class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGItemActionGuideWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(const FText& InActionText, UTexture2D* InKeyIcon, const FVector2D& InKeySize);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> KeyImage;
};
