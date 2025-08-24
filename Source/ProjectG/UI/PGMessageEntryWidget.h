// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGMessageEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMessageEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMessage(FText _Message, float Duration = 0.0f);

protected:
	UPROPERTY(BlueprintReadOnly)
	FText Message;

	FTimerHandle MessageTimerHandle;

	UFUNCTION()
	void ClearMessage();
};
