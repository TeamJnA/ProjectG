// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGIndicatorContainerWidget.generated.h"

class UPGAttributesWidget;
class UPGVoiceIndicatorWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGIndicatorContainerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGAttributesWidget> AttributeWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGVoiceIndicatorWidget> VoiceIndicatorWidget;
	
};
