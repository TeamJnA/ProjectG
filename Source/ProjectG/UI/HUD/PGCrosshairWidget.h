// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGCrosshairWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetCrosshairVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairImage;	
};
