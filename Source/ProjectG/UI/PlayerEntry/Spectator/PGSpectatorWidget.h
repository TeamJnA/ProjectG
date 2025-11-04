// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGSpectatorWidget.generated.h"

class UVerticalBox;
class UPGPlayerEntryWidget;
class APGSpectatorPawn;
class UPGAdvancedFriendsGameInstance;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGSpectatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init();

protected:	
	UFUNCTION()
	void UpdatePlayerList();

	UFUNCTION()
	void HandleSpectateTargetChanged(const APlayerState* NewPlayerState);

	UPROPERTY()
	TArray<TObjectPtr<UPGPlayerEntryWidget>> PlayerEntries;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerListContainer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

private:
	TObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;

};
