// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageManagerWidget.h"
#include "UI/PGMessageEntryWidget.h"
#include "Character/PGPlayerCharacter.h"

void UPGMessageManagerWidget::BindMessageEntry()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller) return;

	APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(Controller->GetPawn());
	if (!PGCharacter) return;

	PlayerRef = PGCharacter;
	PlayerRef->OnStareTargetUpdate.AddDynamic(this, &UPGMessageManagerWidget::HandleOnStareTargetUpdate);
}

void UPGMessageManagerWidget::HandleOnStareTargetUpdate(AActor* TargetActor)
{
	if (TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGMessageManagerWidget::HandleOnStareTargetUpdate: TargetActor Update"));
		MessageEntry->SetMessage(FText::FromString(TargetActor->GetName()));
	}
	else
	{
		MessageEntry->SetMessage(FText::GetEmpty());
	}
}
