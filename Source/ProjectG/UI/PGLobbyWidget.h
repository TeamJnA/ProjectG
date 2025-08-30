// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGLobbyWidget.generated.h"

class UVerticalBox;
class UPGPlayerEntryWidget;

class APGLobbyPlayerController;
class UPGAdvancedFriendsGameInstance;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(APGLobbyPlayerController* PC);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdatePlayerList();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

private:
	TObjectPtr<APGLobbyPlayerController> LocalPC;
	TObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;

};
