// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGMessageManagerWidget.generated.h"

class UPGMessageEntryWidget;
class APGPlayerCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMessageManagerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindMessageEntry(APGPlayerCharacter* PlayerCharacter);
	void ShowFailureMessage(const FText& Message, float Duration);
	
protected:
	UFUNCTION()
	void HandleOnStareTargetUpdate(AActor* TargetActor);
		
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGMessageEntryWidget> MessageEntry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGMessageEntryWidget> FailureMessageEntry;

	TObjectPtr<APGPlayerCharacter> PlayerRef;
};
