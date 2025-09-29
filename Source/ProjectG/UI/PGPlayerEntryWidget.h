// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Type/CharacterTypes.h"

#include "PGPlayerEntryWidget.generated.h"

class UImage;
class UTextBlock;
class APGPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupEntry(const APGPlayerState* InPlayerState, UTexture2D* InAvatarTexture);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerAvatar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;
};
