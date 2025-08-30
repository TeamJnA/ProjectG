// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	// 1. 모든 정보(이름, 호스트 여부, 아바타)를 받는 완전한 버전
	void SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture, bool bIsHostPlayer);

	// 2. 스코어보드용 오버로딩 버전 (호스트 여부 파라미터 없음)
	void SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerAvatar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HostStatusText;
};
