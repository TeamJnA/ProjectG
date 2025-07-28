// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGLobbyWidget.generated.h"

class UButton;
class UTextBlock;
class APGLobbyPlayerController;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(APGLobbyPlayerController* PC);

	UFUNCTION()
	void OnReadyClicked();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyText;

	TObjectPtr<APGLobbyPlayerController> LocalPC;

protected:
	virtual void NativeConstruct() override;
};
