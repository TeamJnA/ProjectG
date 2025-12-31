// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGLobbyWidget.generated.h"

class UVerticalBox;
class UPGPlayerEntryWidget;

class UPGAdvancedFriendsGameInstance;
class APGGameState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void UpdatePlayerList();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

private:
	TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
	TWeakObjectPtr<APGGameState> GSRef;
};
