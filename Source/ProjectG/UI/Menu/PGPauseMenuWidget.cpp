// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGPauseMenuWidget.h"

#include "UI/Menu/PGConfirmWidget.h"
#include "UI/PlayerEntry/Friend/PGFriendListWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameMode.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"

#include "Kismet/GameplayStatics.h"

void UPGPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnResumeButtonClicked);
	}

	if (InviteFriendButton)
	{
		InviteFriendButton->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnInviteFriendButtonClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnOptionButtonClicked);
	}

	if (MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnMainMenuButtonClicked);
	}

	if (DesktopButton)
	{
		DesktopButton->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnDesktopButtonClicked);
	}

	if (BackButton_OptionMenuCanvas)
	{
		BackButton_OptionMenuCanvas->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnBackButtonClicked);
	}

	if (BackButton_InviteMenuCanvas)
	{
		BackButton_InviteMenuCanvas->OnClicked.AddUniqueDynamic(this, &UPGPauseMenuWidget::OnBackButtonClicked);
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

	if (InKeyEvent.GetKey() == EKeys::V)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APGPlayerController* PGPC = Cast<APGPlayerController>(PC))
			{
				PGPC->HandlePushToTalkToggle();
			}
			else if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(PC))
			{
				LobbyPC->HandlePushToTalkToggle();
			}
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
	APlayerController* OwningPC = GetOwningPlayer();

	if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(OwningPC))
	{
		if (LobbyPC->HasAuthority())
		{
			LobbyPC->Server_RequestSessionDestruction(false);
		}
		else
		{
			LobbyPC->Server_RequestSoloLeave(ECleanupActionType::Solo_ReturnToMainMenu);
		}
		return;
	}

	if (APGPlayerController* PGPC = Cast<APGPlayerController>(OwningPC))
	{
		if (PGPC->HasAuthority())
		{
			PGPC->Server_RequestSessionDestruction(false);
		}
		else
		{
			PGPC->Server_RequestSoloLeave(ECleanupActionType::Solo_ReturnToMainMenu);
		}
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
	APlayerController* OwningPC = GetOwningPlayer();

	if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(OwningPC))
	{
		if (LobbyPC->HasAuthority())
		{
			LobbyPC->Server_RequestSessionDestruction(true);
		}
		else
		{
			LobbyPC->Server_RequestSoloLeave(ECleanupActionType::Solo_QuitToDesktop);
		}
		return;
	}

	if (APGPlayerController * PGPC = Cast<APGPlayerController>(GetOwningPlayer()))
	{
		if (PGPC->HasAuthority())
		{
			PGPC->Server_RequestSessionDestruction(true);
		}
		else
		{
			PGPC->Server_RequestSoloLeave(ECleanupActionType::Solo_QuitToDesktop);
		}

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