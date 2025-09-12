// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGPauseMenuWidget.h"

#include "UI/PGConfirmWidget.h"
#include "UI/PGFriendListWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"

#include "Game/PGAdvancedFriendsGameInstance.h"

#include "Kismet/GameplayStatics.h"

void UPGPauseMenuWidget::Init(APlayerController* PC)
{
	OwningPC = PC;	
}

void UPGPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnResumeButtonClicked);
	}

	if (InviteFriendButton)
	{
		InviteFriendButton->OnClicked.AddDynamic(this, &UPGPauseMenuWidget::OnInviteFriendButtonClicked);

		APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
		if (GS && GS->GetCurrentGameState() != EGameState::Lobby)
		{
			InviteFriendButton->SetVisibility(ESlateVisibility::Collapsed);
		}
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

/*
* Resume game
*/
void UPGPauseMenuWidget::OnResumeButtonClicked()
{
	if (OwningPC)
	{
		OwningPC->bShowMouseCursor = false;
		OwningPC->SetInputMode(FInputModeGameOnly());

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

	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnMainMenuButtonClicked: Failed to get GI"));
	}
}

/*
* Leave session and exit game
*/
void UPGPauseMenuWidget::OnDesktopButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnDesktopButtonClicked: Clicked"));

	// Leave session
	UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("PauseMenuWidget::OnDesktopButtonClicked: Failed to get GI"));
	}

	// Exit game
	if (OwningPC)
	{
		UKismetSystemLibrary::QuitGame(this, OwningPC, EQuitPreference::Quit, true);
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