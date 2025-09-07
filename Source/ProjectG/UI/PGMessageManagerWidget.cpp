// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageManagerWidget.h"

#include "UI/PGMessageEntryWidget.h"

#include "Character/PGPlayerCharacter.h"
#include "Interface/InteractableActorInterface.h"

void UPGMessageManagerWidget::BindMessageEntry(APGPlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is valid. Binding delegate."));

	PlayerRef = PlayerCharacter;
	PlayerRef->OnStareTargetUpdate.AddDynamic(this, &UPGMessageManagerWidget::HandleOnStareTargetUpdate);
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: Delegate bound to OnStareTargetUpdate on character: %s"), *PlayerRef->GetName()); //
}

void UPGMessageManagerWidget::ShowFailureMessage(const FText& Message, float Duration)
{
	if (FailureMessageEntry)
	{
		FailureMessageEntry->SetMessage(Message, Duration);
	}
}

void UPGMessageManagerWidget::HandleOnStareTargetUpdate(AActor* TargetActor)
{
	if (TargetActor)
	{
		if (IInteractableActorInterface* Interactable = Cast<IInteractableActorInterface>(TargetActor))
		{
			const FInteractionInfo Info = Interactable->GetInteractionInfo();
			FText MessageToShow;

			if (Info.InteractionType == EInteractionType::Hold)
			{
				MessageToShow = FText::FromString(TEXT("Hold F to Interact"));
			}
			else
			{
				MessageToShow = FText::FromString(TEXT("Press F to Interact"));
			}

			if (MessageEntry)
			{
				MessageEntry->SetMessage(MessageToShow);
			}
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("UPGMessageManagerWidget::HandleOnStareTargetUpdate: TargetActor is NULL, clearing message."));
		if (MessageEntry) // MessageEntry 널 체크 추가
		{
			MessageEntry->SetMessage(FText::GetEmpty());
		}
		if (FailureMessageEntry)
		{
			FailureMessageEntry->SetMessage(FText::GetEmpty());
		}
	}
}
