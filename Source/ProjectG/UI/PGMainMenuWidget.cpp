// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGMainMenuWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/PGSessionSlotWidget.h"
#include "UI/PGConfirmWidget.h"
#include "UI/PGSettingMenuWidget.h"
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

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnOptionButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnExitButtonClicked);
	}
	//if (RefreshButton)
	//{
	//	RefreshButton->OnClicked.AddDynamic(this, &UPGLobbyWidget::OnRefreshClicked);
	//}
}

void UPGMainMenuWidget::OnExitButtonClicked()
{
	if (ConfirmWidgetInstance && ConfirmWidgetInstance->IsInViewport())
	{
		return;
	}

	if (ConfirmWidgetClass)
	{
		ConfirmWidgetInstance = CreateWidget<UPGConfirmWidget>(this, ConfirmWidgetClass);
		ConfirmWidgetInstance->SetOwningPlayerController(CachedPC);
		if (ConfirmWidgetInstance)
		{
			ConfirmWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnExitButtonClicked: Exit Confirm pop-up added"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget::OnExitButtonClicked: ConfirmWidget class is null"));
	}

}

void UPGMainMenuWidget::OnOptionButtonClicked()
{
	if (SettingMenuWidgetInstance && SettingMenuWidgetInstance->IsInViewport())
	{
		return;
	}

	if (SettingMenuWidgetClass)
	{
		SettingMenuWidgetInstance = CreateWidget<UPGSettingMenuWidget>(this, SettingMenuWidgetClass);
		if (SettingMenuWidgetInstance)
		{
			SettingMenuWidgetInstance->AddToViewport();
			UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnOptionButtonClicked: Option menu widget added"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget::OnOptionButtonClicked: SettingMenuWidget class is null"));
	}
}
