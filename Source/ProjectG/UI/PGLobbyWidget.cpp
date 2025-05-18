// Fill out your copyright notice in the Description page of Project Settings.


#include "PGLobbyWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/PGSessionSlotWidget.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PGLobbyPlayerController.h"

void UPGLobbyWidget::AddSessionSlot(const FString& ServerName, int32 Index)
{
	if (!SessionListContainer || !SessionSlotWidgetClass) return;

	UPGSessionSlotWidget* slot = CreateWidget<UPGSessionSlotWidget>(this, SessionSlotWidgetClass);
	if (slot)
	{
		if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
		{
			slot->Setup(ServerName, Index, GI);
		}
		SessionListContainer->AddChild(slot);
	}
}

void UPGLobbyWidget::ClearSessionList()
{
	if (SessionListContainer)
	{
		SessionListContainer->ClearChildren();
	}
}

void UPGLobbyWidget::OnReadyClicked()
{
	APGLobbyPlayerController* PC = Cast<APGLobbyPlayerController>(CachedPC);
	if (PC)
	{
		PC->SetReady();
	}

	ReadyButton->SetIsEnabled(false);
	ReadyText->SetText(FText::FromString(TEXT("Waiting...")));
}

//void UPGLobbyWidget::OnRefreshClicked()
//{
//	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
//	{
//		GI->FindSessions();	
//	}
//}

void UPGLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedPC = UGameplayStatics::GetPlayerController(this, 0);
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UPGLobbyWidget::OnReadyClicked);
	}
	//if (RefreshButton)
	//{
	//	RefreshButton->OnClicked.AddDynamic(this, &UPGLobbyWidget::OnRefreshClicked);
	//}
}
