// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGIndicatorContainerWidget.h"
#include "UI/HUD/PGAttributesWidget.h"


void UPGIndicatorContainerWidget::Init()
{
	if (AttributeWidget)
	{
		AttributeWidget->BindToAttributes();
	}
}
