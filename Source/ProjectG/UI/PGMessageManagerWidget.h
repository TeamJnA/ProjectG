// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGMessageManagerWidget.generated.h"

class UPGMessageEntryWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMessageManagerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindMessageEntry(AActor* InteractableActor);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGMessageEntryWidget> MessageEntry;
};
