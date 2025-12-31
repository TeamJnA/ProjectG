// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGMainMenuWidget.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

#include "UI/Menu/PGSessionSlotWidget.h"
#include "UI/Menu/PGConfirmWidget.h"
#include "UI/Menu/PGSessionStatusWidget.h"
#include "UI/Menu/PGSettingMenuWidget.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Player/PGLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UPGMainMenuWidget::NativeOnInitialized()
{
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnHostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnJoinButtonClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnOptionButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnExitButtonClicked);
	}

	if (OptionMenuCanvas_BackButton)
	{
		OptionMenuCanvas_BackButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnBackButtonClicked);
	}

	if (SessionListCanvas_BackButton)
	{
		SessionListCanvas_BackButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnBackButtonClicked);
	}

	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UPGMainMenuWidget::OnRefreshButtonClicked);
	}

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;

		GI->OnSessionsFound.AddUObject(this, &UPGMainMenuWidget::OnSessionsFound);

		GI->OnHostSessionAttemptStarted.AddDynamic(this, &UPGMainMenuWidget::HandleHostSessionStarted);
		GI->OnHostSessionAttemptFinished.AddDynamic(this, &UPGMainMenuWidget::HandleHostSessionFinished);
		GI->OnFindSessionAttemptStarted.AddDynamic(this, &UPGMainMenuWidget::HandleFindSessionStarted);
		GI->OnFindSessionAttemptFinished.AddDynamic(this, &UPGMainMenuWidget::HandleFindSessionFinished);
		GI->OnJoinSessionAttemptStarted.AddDynamic(this, &UPGMainMenuWidget::HandleJoinSessionStarted);
		GI->OnJoinSessionAttemptFinished.AddDynamic(this, &UPGMainMenuWidget::HandleJoinSessionFinished);
	}
}

void UPGMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();

	SetMainMenuButtonEnabled(true);	
}

void UPGMainMenuWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SessionStatusWidgetTimerHandle);
	}

	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		GI->OnSessionsFound.RemoveAll(this);
		GI->OnHostSessionAttemptStarted.RemoveAll(this);
		GI->OnHostSessionAttemptFinished.RemoveAll(this);
		GI->OnFindSessionAttemptStarted.RemoveAll(this);
		GI->OnFindSessionAttemptFinished.RemoveAll(this);
		GI->OnJoinSessionAttemptStarted.RemoveAll(this);
		GI->OnJoinSessionAttemptFinished.RemoveAll(this);
	}

	Super::NativeDestruct();
}

FReply UPGMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		const int32 CurrentIndex = WidgetSwitcher->GetActiveWidgetIndex();

		if (CurrentIndex == 0)
		{
			OnExitButtonClicked();
		}
		else
		{
			OnBackButtonClicked();
		}

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

/*
* 세션 슬롯 생성부
* 세션 생성/Setup
*/
void UPGMainMenuWidget::AddSessionSlot(const FOnlineSessionSearchResult& SearchResult, int32 Index)
{
	if (!SessionListContainer || !SessionSlotWidgetClass)
	{
		return;
	}

	UPGSessionSlotWidget* SessionSlot = CreateWidget<UPGSessionSlotWidget>(this, SessionSlotWidgetClass);
	if (SessionSlot)
	{
		if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
		{
			SessionSlot->Setup(SearchResult, Index, GI);
		}
		SessionListContainer->AddChild(SessionSlot);
	}
}

/*
* 세션 슬롯 컨테이너 초기화
*/
void UPGMainMenuWidget::ClearSessionList()
{
	if (SessionListContainer)
	{
		SessionListContainer->ClearChildren();
	}
}

void UPGMainMenuWidget::OnHostButtonClicked()
{
	if (!ConfirmWidgetClass)
	{
		return;
	}

	if (SessionStatusWidgetInstance && SessionStatusWidgetInstance->IsInViewport())
	{
		return;
	}

	if (!ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance = CreateWidget<UPGConfirmWidget>(this, ConfirmWidgetClass);
	}

	if (ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance->SetConfirmText(FText::FromString(TEXT("Create Session?")));
		ConfirmWidgetInstance->SetReturnFocusWidget(this);
		ConfirmWidgetInstance->OnConfirmClicked.RemoveAll(this);
		ConfirmWidgetInstance->OnConfirmClicked.AddDynamic(this, &UPGMainMenuWidget::StartHostSession);
		if (!ConfirmWidgetInstance->IsInViewport())
		{
			ConfirmWidgetInstance->AddToViewport();
		}
	}
}

void UPGMainMenuWidget::StartHostSession()
{
	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		GI->HostSession(NAME_GameSession, 4, false);
	}
}

void UPGMainMenuWidget::OnJoinButtonClicked()
{
	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnJoinButtonClicked: Join button clicked"));
		if (WidgetSwitcher)
		{
			WidgetSwitcher->SetActiveWidgetIndex(2);
		}

		ClearSessionList();
		GI->FindSessions();
	}
}

void UPGMainMenuWidget::OnExitButtonClicked()
{
	if (!ConfirmWidgetClass)
	{
		return;
	}

	if (SessionStatusWidgetInstance && SessionStatusWidgetInstance->IsInViewport())
	{
		return;
	}

	if (!ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance = CreateWidget<UPGConfirmWidget>(this, ConfirmWidgetClass);
	}

	if (ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance->SetConfirmText(FText::FromString(TEXT("Exit Game?")));
		ConfirmWidgetInstance->SetReturnFocusWidget(this);
		ConfirmWidgetInstance->OnConfirmClicked.RemoveAll(this);
		ConfirmWidgetInstance->OnConfirmClicked.AddDynamic(this, &UPGMainMenuWidget::QuitGame);
		if (!ConfirmWidgetInstance->IsInViewport())
		{
			ConfirmWidgetInstance->AddToViewport();
		}
	}
}

void UPGMainMenuWidget::QuitGame()
{
	if (!GetOwningPlayer())
	{
		return;
	}
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
}

void UPGMainMenuWidget::OnOptionButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}
}

void UPGMainMenuWidget::OnRefreshButtonClicked()
{
	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnRefreshButtonClicked: Refresh Button Clicked. Finding sessions..."));
		ClearSessionList();
		GI->FindSessions();
	}
}

void UPGMainMenuWidget::OnBackButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
	}
}

void UPGMainMenuWidget::HandleHostSessionStarted()
{
	ShowSessionStatusWidget(FText::FromString(TEXT("Creating Session")), false);
	SetMainMenuButtonEnabled(false);
	SetSessionListButtonEnabled(false);
}

void UPGMainMenuWidget::HandleHostSessionFinished(bool bWasSuccessful, const FText& ErrorMessage)
{
	if (!bWasSuccessful)
	{
		ShowSessionStatusWidget(ErrorMessage, true);
		HideSessionStatusWidget(5.0f);
		SetMainMenuButtonEnabled(true);
		SetSessionListButtonEnabled(true);
	}
}

/*
* GameInstance로부터 검색된 세션 목록을 전달받아 세션 슬롯 생성
*/
void UPGMainMenuWidget::OnSessionsFound(const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	ClearSessionList();

	if (SessionResults.Num() == 0)
	{
		ShowSessionStatusWidget(FText::FromString(TEXT("No sessions found")), true);
		HideSessionStatusWidget(5.0f);
		UE_LOG(LogTemp, Log, TEXT("MainMenuWidget::OnSessionsFound: No sessions found"));
	}
	else
	{
		HideSessionStatusWidget();
		for (int32 i = 0; i < SessionResults.Num(); ++i)
		{
			const FOnlineSessionSearchResult& Result = SessionResults[i];

			AddSessionSlot(Result, i);
		}
	}

	SetMainMenuButtonEnabled(true);
	SetSessionListButtonEnabled(true);
}

void UPGMainMenuWidget::HandleFindSessionStarted()
{
	ShowSessionStatusWidget(FText::FromString(TEXT("Finding Sessions")), false);
	SetMainMenuButtonEnabled(false);
	SetSessionListButtonEnabled(false);
}

void UPGMainMenuWidget::HandleFindSessionFinished(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ShowSessionStatusWidget(FText::FromString(TEXT("Failed to find sessions")), true);
		HideSessionStatusWidget(2.0f);
	}
	else
	{
		HideSessionStatusWidget();
	}
}

void UPGMainMenuWidget::HandleJoinSessionStarted()
{
	ShowSessionStatusWidget(FText::FromString(TEXT("Joining Session")), false);
	SetMainMenuButtonEnabled(false);
	SetSessionListButtonEnabled(false);
}

void UPGMainMenuWidget::HandleJoinSessionFinished(bool bWasSuccessful, const FText& ErrorMessage)
{
	if (!bWasSuccessful)
	{
		ShowSessionStatusWidget(ErrorMessage, true);
		HideSessionStatusWidget(5.0f);
		SetMainMenuButtonEnabled(true);
		SetSessionListButtonEnabled(true);
	}
}

void UPGMainMenuWidget::SetMainMenuButtonEnabled(bool bEnabled)
{
	if (HostButton)
	{
		HostButton->SetIsEnabled(bEnabled);
	}
	
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(bEnabled);
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(bEnabled);
	}

	if (OptionButton)
	{
		OptionButton->SetIsEnabled(bEnabled);
	}

	if (ExitButton)
	{
		ExitButton->SetIsEnabled(bEnabled);
	}

	if (OptionMenuCanvas_BackButton)
	{
		OptionMenuCanvas_BackButton->SetIsEnabled(bEnabled);
	}

	if (SessionListCanvas_BackButton)
	{
		SessionListCanvas_BackButton->SetIsEnabled(bEnabled);
	}
}

void UPGMainMenuWidget::SetSessionListButtonEnabled(bool bEnabled)
{
	if (!SessionListContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("MainMenuWidget::SetSessionListButtonEnabled: Session list container is not valid"));
		return;
	}

	for (UWidget* Child : SessionListContainer->GetAllChildren())
	{
		UPGSessionSlotWidget* SessionSlot = Cast<UPGSessionSlotWidget>(Child);
		if (!SessionSlot)
		{
			continue;
		}

		if (!SessionSlot->JoinButton)
		{
			continue;
		}

		SessionSlot->JoinButton->SetIsEnabled(bEnabled);
	}
}

void UPGMainMenuWidget::ShowSessionStatusWidget(const FText& Message, bool bShowCloseButton)
{
	GetWorld()->GetTimerManager().ClearTimer(SessionStatusWidgetTimerHandle);

	if (!SessionStatusWidgetClass)
	{
		return;
	}

	if (!SessionStatusWidgetInstance)
	{
		SessionStatusWidgetInstance = CreateWidget<UPGSessionStatusWidget>(this, SessionStatusWidgetClass);
	}

	if (SessionStatusWidgetInstance)
	{
		SessionStatusWidgetInstance->SetStatusMessage(Message, bShowCloseButton);
		SessionStatusWidgetInstance->SetReturnFocusWidget(this);
		if (!SessionStatusWidgetInstance->IsInViewport())
		{
			SessionStatusWidgetInstance->AddToViewport(10);
		}
	}
}

void UPGMainMenuWidget::HideSessionStatusWidget(float Delay)
{
	if (Delay > 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(SessionStatusWidgetTimerHandle);

		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, FName("HideSessionStatusWidget"), 0.0f);
		GetWorld()->GetTimerManager().SetTimer(SessionStatusWidgetTimerHandle, Delegate, Delay, false);
	}
	else
	{
		if (SessionStatusWidgetInstance && SessionStatusWidgetInstance->IsInViewport())
		{
			SessionStatusWidgetInstance->RemoveFromParent();
			SetKeyboardFocus();
		}
		GetWorld()->GetTimerManager().ClearTimer(SessionStatusWidgetTimerHandle);
	}
}
