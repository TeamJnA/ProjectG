// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGScoreBoardWidget.generated.h"

class UPGPlayerEntryWidget;
class APGPlayerCharacter;
class UVerticalBox;
class UButton;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGScoreBoardWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void BindPlayerEntry(APlayerController* _PC);

	UFUNCTION()
	void UpdatePlayerEntry();

protected:
	virtual void NativeConstruct() override;

	TObjectPtr<APlayerController> PCRef;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UPGPlayerEntryWidget> PlayerEntryWidgetClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SpectateButton;

	UFUNCTION()
	void OnSpectateButtonClicked();
};
