// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/ScoreBoard/PGFinalScoreBoardWidget.h"
#include "UI/PlayerEntry/PGPlayerEntryWidget.h"
#include "UI/Menu/PGConfirmWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Character/PGPlayerCharacter.h"

#include "Kismet/GameplayStatics.h"


void UPGFinalScoreBoardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked);
	}

	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToLobbyButtonClicked);
	}

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GSRef = GS;
		GS->OnPlayerArrayChanged.RemoveAll(this);
		GS->OnPlayerArrayChanged.AddDynamic(this, &UPGFinalScoreBoardWidget::UpdatePlayerEntry);
	}
}

void UPGFinalScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();
}

void UPGFinalScoreBoardWidget::NativeDestruct()
{
	if (APGGameState* GS = GSRef.Get())
	{
		GS->OnPlayerArrayChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

/*
* 로컬 플레이어에 FinalScoreBoardWidget 생성 이후 플레이어 목록 업데이트
* GameState의 PlayerList 업데이트 시 전달되는 OnPlayerListUpdated 델리게이트에 업데이트 함수 바인드
*/
void UPGFinalScoreBoardWidget::BindPlayerEntry()
{
	UE_LOG(LogTemp, Log, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is valid. Binding delegate."));

	UpdatePlayerEntry();
}

/*
* 플레이어 목록 업데이트 구현부
* 모든 플레이어들의 Steam 프로필 이미지, 이름, 종료 상태 디스플레이
*/
void UPGFinalScoreBoardWidget::UpdatePlayerEntry()
{
	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	APGGameState* GS = GSRef.Get();
	if (!GI || !GS)
	{
		return;
	}

	PlayerContainer->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			UPGPlayerEntryWidget* NewSlot = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewSlot)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PGPS->GetUniqueId().IsValid())
				{
					AvatarTexture = GI->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
				}

				NewSlot->SetupEntry(PGPS, AvatarTexture, EPlayerEntryContext::Scoreboard);
				PlayerContainer->AddChild(NewSlot);
				UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Add PlayerEntry | Name: %s"), *PGPS->GetPlayerName());
			}
		}		
	}
}

/*
* 메인 메뉴 버튼 -> 세션 종료 후 메인 메뉴로 이동
*/
void UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToMainMenuButtonClicked: Clicked"));

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
		ConfirmWidgetInstance->OnConfirmClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::ReturnToMainMenu);
		if (!ConfirmWidgetInstance->IsInViewport())
		{
			ConfirmWidgetInstance->AddToViewport();
		}
	}
}

void UPGFinalScoreBoardWidget::ReturnToMainMenu()
{
	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		GI->LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::OnReturnToMainMenuButtonClicked: No valid GI"));
	}
}

/*
* 로비로 나가기 버튼 -> 세션을 유지한 채 로비로 이동
* 다른 플레이어들의 선택을 기다린 후 
* 로비로 나가기 버튼을 클릭한 플레이어들에 대해 세션을 유지한 채 로비로 이동
*/
void UPGFinalScoreBoardWidget::OnReturnToLobbyButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: Clicked"));

	if (APGPlayerController* PC = Cast<APGPlayerController>(GetOwningPlayer()))
	{
		PC->NotifyReadyToReturnLobby();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: PC Ref is not PG class"));
	}
}
