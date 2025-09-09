// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGFinalScoreBoardWidget.generated.h"

class APGGameState;
class UPGPlayerEntryWidget;
class APGPlayerCharacter;
class UVerticalBox;
class UButton;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGFinalScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindPlayerEntry(APlayerController* _PC);

	UFUNCTION()
	void UpdatePlayerEntry();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnReturnToMainMenuButtonClicked();

	UFUNCTION()
	void OnReturnToLobbyButtonClicked();

	TObjectPtr<APlayerController> PCRef;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToMainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnToLobbyButton;

private:
	TWeakObjectPtr<APGGameState> GSRef;
};
