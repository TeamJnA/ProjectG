// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPlayerProfileWidgetBase.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class APGPlayerState;
class UPGAdvancedFriendsGameInstance;

/**
 * 
 */
UCLASS(Abstract)
class PROJECTG_API UPGPlayerProfileWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	bool ResolveGameInstance();
	void ApplyAvatarAndName(APGPlayerState* LocalPlayerState);
	void ApplyProgressForXP(int64 DisplayXP);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ProfileAvatar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RankNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> XPText;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> RankUpAnim;
	
	TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
	int32 LastShownRankIndex = 0;
};
