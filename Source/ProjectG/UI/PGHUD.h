// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PGHUD.generated.h"

class UPGAttributesWidget;
class UPGInventoryWidget;
class UPGInventoryComponent;
/**
 * 
 */
UCLASS()
class PROJECTG_API APGHUD : public AHUD
{
	GENERATED_BODY()

public:
	void Init();
	
protected:
	APGHUD();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGAttributesWidget> AttributeWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPGInventoryWidget> InventoryWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UPGAttributesWidget> AttributeWidget;

	UPROPERTY()
	TObjectPtr<UPGInventoryWidget> InventoryWidget;
};
