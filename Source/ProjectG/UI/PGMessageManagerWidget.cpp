// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageManagerWidget.h"
#include "UI/PGMessageEntryWidget.h"
#include "Character/PGPlayerCharacter.h"

void UPGMessageManagerWidget::BindMessageEntry(APGPlayerCharacter* PlayerCharacter)
{
	// GetOwningPlayer()�� GetPawn()���� ĳ���͸� ã�� �ʿ� ����
	// APlayerController* Controller = GetOwningPlayer();
	// if (!Controller) return;
	// APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(Controller->GetPawn());

	if (!PlayerCharacter) // ���ڷ� ���� ĳ���� �����Ͱ� ��ȿ���� Ȯ��
	{
		UE_LOG(LogTemp, Error, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is valid. Binding delegate."));

	PlayerRef = PlayerCharacter;
	PlayerRef->OnStareTargetUpdate.AddDynamic(this, &UPGMessageManagerWidget::HandleOnStareTargetUpdate);
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: Delegate bound to OnStareTargetUpdate on character: %s"), *PlayerRef->GetName()); //
}

void UPGMessageManagerWidget::HandleOnStareTargetUpdate(AActor* TargetActor)
{
	if (TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPGMessageManagerWidget::HandleOnStareTargetUpdate: TargetActor Update"));
		if (MessageEntry) // MessageEntry �� üũ �߰� (������ ��ȭ)
		{
			MessageEntry->SetMessage(FText::FromString(TargetActor->GetName()));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UPGMessageManagerWidget::HandleOnStareTargetUpdate: MessageEntry is NULL!"));
		}
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("UPGMessageManagerWidget::HandleOnStareTargetUpdate: TargetActor is NULL, clearing message."));
		if (MessageEntry) // MessageEntry �� üũ �߰�
		{
			MessageEntry->SetMessage(FText::GetEmpty());
		}
	}
}
