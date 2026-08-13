// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGInteractionProgressWidget.h"
#include "Components/Image.h"


void UPGInteractionProgressWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ProgressImage)
	{
		ProgressMID = ProgressImage->GetDynamicMaterial();
	}
}

void UPGInteractionProgressWidget::SetProgress(float InProgress)
{
	Progress = InProgress;

	if (ProgressMID)
	{
		ProgressMID->SetScalarParameterValue(TEXT("Progress"), Progress);
	}
}