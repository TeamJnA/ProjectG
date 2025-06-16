// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGAttributesWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAttributesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindToAttributes();

protected:
	UPROPERTY(BlueprintReadOnly)
	float StaminaPercent;	
};
