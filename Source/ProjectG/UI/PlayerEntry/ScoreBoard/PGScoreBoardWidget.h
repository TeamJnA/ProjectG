// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGScoreBoardWidget.generated.h"

class UPGPlayerEntryWidget;
class UVerticalBox;
class UButton;
class UPGAdvancedFriendsGameInstance;
class APGGameState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void BindPlayerEntry();

	UFUNCTION()
	void UpdatePlayerEntry();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SpectateButton;

	UFUNCTION()
	void OnSpectateButtonClicked();

private:
	TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
	TWeakObjectPtr<APGGameState> GSRef;
};
