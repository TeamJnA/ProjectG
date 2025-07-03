// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMessageManagerWidget.h"
#include "UI/PGMessageEntryWidget.h"
#include "Character/PGPlayerCharacter.h"

void UPGMessageManagerWidget::BindMessageEntry(APGPlayerCharacter* PlayerCharacter)
{
	// GetOwningPlayer()나 GetPawn()으로 캐릭터를 찾을 필요 없음
	// APlayerController* Controller = GetOwningPlayer();
	// if (!Controller) return;
	// APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(Controller->GetPawn());

	if (!PlayerCharacter) // 인자로 받은 캐릭터 포인터가 유효한지 확인
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
		if (MessageEntry) // MessageEntry 널 체크 추가 (안전성 강화)
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
		if (MessageEntry) // MessageEntry 널 체크 추가
		{
			MessageEntry->SetMessage(FText::GetEmpty());
		}
	}
}
