// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGInteractionProgressWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGInteractionProgressWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetProgress(float InProgress);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ProgressImage;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ProgressMID;

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	float Progress;
};
