// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMainMenuWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/PGSessionSlotWidget.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PGLobbyPlayerController.h"

void UPGMainMenuWidget::AddSessionSlot(const FString& ServerName, int32 Index)
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

void UPGMainMenuWidget::ClearSessionList()
{
	if (SessionListContainer)
	{
		SessionListContainer->ClearChildren();
	}
}

//void UPGMainMenuWidget::OnRefreshClicked()
//{
//	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
//	{
//		GI->FindSessions();	
//	}
//}

void UPGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedPC = UGameplayStatics::GetPlayerController(this, 0);

	//if (RefreshButton)
	//{
	//	RefreshButton->OnClicked.AddDynamic(this, &UPGLobbyWidget::OnRefreshClicked);
	//}
}
