// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	void Setup(const FString& SessionName, int32 SessionIndex, UPGAdvancedFriendsGameInstance* GI);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SessionNameText;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void OnJoinClicked();

private:
	int32 Index;
	UPGAdvancedFriendsGameInstance* GameInstanceRef;
};
