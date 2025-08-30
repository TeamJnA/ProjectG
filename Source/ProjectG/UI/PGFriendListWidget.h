// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGFriendListWidget.generated.h"

class UVerticalBox;
class UPGAdvancedFriendsGameInstance;
class UPGFriendEntryWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGFriendListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshFriendList();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnFriendListUpdated();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> FriendListContainer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPGFriendEntryWidget> FriendEntryWidgetClass;

private:
	TObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
};
