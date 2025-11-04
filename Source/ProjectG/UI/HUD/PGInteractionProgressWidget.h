// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInteractionProgressWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGInteractionProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetProgress(float _Progress);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float Progress;
	
};
