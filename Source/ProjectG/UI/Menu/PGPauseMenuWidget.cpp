// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGPauseMenuWidget.h"

#include "UI/Menu/PGConfirmWidget.h"
#include "UI/PlayerEntry/Friend/PGFriendListWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Kismet/GameplayStatics.h"

void UPGPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnResumeButtonClicked);
	}

	if (InviteFriendButton)
	{
		InviteFriendButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnInviteFriendButtonClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnOptionButtonClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnMainMenuButtonClicked);
	}

	if (DesktopButton)
	{
		DesktopButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnDesktopButtonClicked);
	}

	if (BackButton_OptionMenuCanvas)
	{
		BackButton_OptionMenuCanvas->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnBackButtonClicked);
	}

	if (BackButton_InviteMenuCanvas)
	{
		BackButton_InviteMenuCanvas->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnBackButtonClicked);
	}
}

void UPGPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();

	if (InviteFriendButton)
	{
		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (GS && GS->GetCurrentGameState() != EGameState::Lobby)
		{
			InviteFriendButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FReply UPGPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		const int32 CurrentIndex = WidgetSwitcher->GetActiveWidgetIndex();
		if (CurrentIndex == 0)
		{
			OnResumeButtonClicked();
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
* Resume game
*/
void UPGPauseMenuWidget::OnResumeButtonClicked()
{
	if (GetOwningPlayer())
	{
		GetOwningPlayer()->bShowMouseCursor = false;
		GetOwningPlayer()->SetInputMode(FInputModeGameOnly());
		
		RemoveFromParent();
	}
}

void UPGPauseMenuWidget::OnInviteFriendButtonClicked()
{
	if (WidgetSwitcher && FriendListWidget)
	{
		FriendListWidget->RefreshFriendList();
		WidgetSwitcher->SetActiveWidgetIndex(2);
	}
}

/*
* Pause menu widget -> Setting widget
*/
void UPGPauseMenuWidget::OnOptionButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(1);
	}
}

/*
* Leave session and return to MainMenu
*/
void UPGPauseMenuWidget::OnMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnMainMenuButtonClicked: Clicked"));

	if (!ConfirmWidgetClass)
	{
		return;
	}

	if (!ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance = CreateWidget<UPGConfirmWidget>(this, ConfirmWidgetClass);
	}

	if (ConfirmWidgetInstance)
	{
		ConfirmWidgetInstance->SetConfirmText(FText::FromString(TEXT("Return To MainMenu?")));
		ConfirmWidgetInstance->SetReturnFocusWidget(this);
		ConfirmWidgetInstance->OnConfirmClicked.RemoveAll(this);
		ConfirmWidgetInstance->OnConfirmClicked.AddDynamic(this, &UPGPauseMenuWidget::ReturnToMainMenu);
		if (!ConfirmWidgetInstance->IsInViewport())
		{
			ConfirmWidgetInstance->AddToViewport();
		}
	}
}

void UPGPauseMenuWidget::ReturnToMainMenu()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (GI)
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::ReturnToMainMenu: Failed to get GI"));
	}
}

/*
* Leave session and exit game
*/
void UPGPauseMenuWidget::OnDesktopButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnDesktopButtonClicked: Clicked"));

	if (!ConfirmWidgetClass)
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
		ConfirmWidgetInstance->OnConfirmClicked.AddDynamic(this, &UPGPauseMenuWidget::ReturnToDesktop);
		if (!ConfirmWidgetInstance->IsInViewport())
		{
			ConfirmWidgetInstance->AddToViewport();
		}
	}
}

void UPGPauseMenuWidget::ReturnToDesktop()
{
	// Leave session
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (GI)
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnDesktopButtonClicked: Failed to get GI"));
	}

	// Exit game
	if (GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
	}
}

/*
* Setting widget -> Pause menu widget
*/
void UPGPauseMenuWidget::OnBackButtonClicked()
{
	if (WidgetSwitcher)
	{
		WidgetSwitcher->SetActiveWidgetIndex(0);
	}
}