// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPlayerEntryWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class APGPlayerState;

UENUM()
enum class EPlayerEntryContext : uint8
{
	Lobby,
	Scoreboard,
	Spectator
};

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupEntry(const APGPlayerState* InPlayerState, UTexture2D* InAvatarTexture, EPlayerEntryContext Context = EPlayerEntryContext::Lobby);

	void HighlightEntry();
	void UnhighlightEntry();

	const APlayerState* GetPlayerState() const { return PlayerStateRef; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerAvatar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> HighlightBorder;

private:
	UPROPERTY()
	TObjectPtr<APlayerState> PlayerStateRef;

	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor HighlightColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UI")
	FLinearColor DefaultColor = FLinearColor::Transparent;

};
