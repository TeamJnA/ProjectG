// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Interfaces/OnlineSessionInterface.h"

#include "PGSessionSlotWidget.generated.h"

class UTextBlock;
class UButton;
class UPGAdvancedFriendsGameInstance;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGSessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(const FOnlineSessionSearchResult& SearchResult, int32 SessionIndex, UPGAdvancedFriendsGameInstance* GI);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SessionNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PingText;

	UFUNCTION()
	void OnJoinClicked();

private:
	int32 Index;
};
