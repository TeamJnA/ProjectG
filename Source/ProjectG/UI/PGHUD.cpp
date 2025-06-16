// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGHUD.h"
#include "UI/PGAttributesWidget.h"
#include "UI/PGInventoryWidget.h"
#include "UI/PGMessageManagerWidget.h"
#include "Character/Component/PGInventoryComponent.h"

APGHUD::APGHUD()
{
}

void APGHUD::Init()
{
	AttributeWidget = CreateWidget<UPGAttributesWidget>(GetOwningPlayerController(), AttributeWidgetClass);
	AttributeWidget->BindToAttributes();
	AttributeWidget->AddToViewport();

	InventoryWidget = CreateWidget<UPGInventoryWidget>(GetOwningPlayerController(), InventoryWidgetClass);
	InventoryWidget->BindInventorySlots();
	InventoryWidget->AddToViewport();

	MessageManagerWidget = CreateWidget<UPGMessageManagerWidget>(GetOwningPlayerController(), MessageManagerWidgetClass);
	// bind
	MessageManagerWidget->AddToViewport();
}
