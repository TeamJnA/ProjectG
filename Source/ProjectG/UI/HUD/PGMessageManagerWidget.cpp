// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGMessageManagerWidget.h"

#include "UI/HUD/PGMessageEntryWidget.h"

#include "Character/PGPlayerCharacter.h"
#include "Interface/InteractableActorInterface.h"

/*
* 플레이어 메시지 위젯 바인드
* 바라보는 대상에 따른 메시지 디스플레이
*/
void UPGMessageManagerWidget::BindMessageEntry(APGPlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is valid. Binding delegate."));

	PlayerRef = PlayerCharacter;
	PlayerRef->OnStareTargetUpdate.AddDynamic(this, &UPGMessageManagerWidget::HandleOnStareTargetUpdate);
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
		if (MessageEntry)
		{
			MessageEntry->SetMessage(FText::GetEmpty());
		}
		if (FailureMessageEntry)
		{
			FailureMessageEntry->SetMessage(FText::GetEmpty());
		}
	}
}
