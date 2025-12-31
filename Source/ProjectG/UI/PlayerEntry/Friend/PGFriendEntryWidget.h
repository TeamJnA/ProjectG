// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGFriendEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UPGAdvancedFriendsGameInstance;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGFriendEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupFriendEntry(const FText& InFriendName, UTexture2D* InAvatarTexture, bool bIsOnline, TSharedPtr<const FUniqueNetId> InFriendNetId);

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void OnInviteButtonClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FriendAvatar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FriendNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InviteButton;

private:
	TSharedPtr<const FUniqueNetId> FriendNetId;
};
