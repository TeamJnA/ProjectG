// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGPlayerEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPlayerEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPlayerText(FText _Player) { Player = _Player; }

protected:
	UPROPERTY(BlueprintReadOnly)
	FText Player;
};
