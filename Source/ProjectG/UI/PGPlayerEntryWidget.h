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
	// 1. ��� ����(�̸�, ȣ��Ʈ ����, �ƹ�Ÿ)�� �޴� ������ ����
	void SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture, bool bIsHostPlayer);

	// 2. ���ھ��� �����ε� ���� (ȣ��Ʈ ���� �Ķ���� ����)
	void SetupEntry(const FText& InPlayerName, UTexture2D* InAvatarTexture);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerAvatar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HostStatusText;
};
