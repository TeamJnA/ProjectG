// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGFinalScoreBoardWidget.h"

#include "Kismet/GameplayStatics.h"

#include "UI/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Game/PGGameState.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Character/PGPlayerCharacter.h"

/*
* 로컬 플레이어에 FinalScoreBoardWidget 생성 이후 플레이어 목록 업데이트
* GameState의 PlayerList 업데이트 시 전달되는 OnPlayerListUpdated 델리게이트에 업데이트 함수 바인드
*/
void UPGFinalScoreBoardWidget::BindPlayerEntry(APlayerController* InPC)
{
	if (!InPC)
	{
		UE_LOG(LogTemp, Error, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is NULL! Cannot bind delegate."));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGFinalScoreBoardWidget::BindPlayerEntry: InPC is valid. Binding delegate."));

	PCRef = InPC;

	UpdatePlayerEntry();
}

/*
* 플레이어 목록 업데이트 구현부
* 모든 플레이어들의 Steam 프로필 이미지, 이름, 종료 상태 디스플레이
*/
void UPGFinalScoreBoardWidget::UpdatePlayerEntry()
{
	if (!GetWorld())
	{
		return;
	}

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GS || !GI)
	{
		return;
	}

	PlayerContainer->ClearChildren();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (const APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
		{
			UPGPlayerEntryWidget* NewSlot = CreateWidget<UPGPlayerEntryWidget>(this, PlayerEntryWidgetClass);
			if (NewSlot)
			{
				UTexture2D* AvatarTexture = nullptr;
				if (PGPS->GetUniqueId().IsValid())
				{
					AvatarTexture = GI->GetSteamAvatarAsTexture(*PGPS->GetUniqueId().GetUniqueNetId());
				}

				NewSlot->SetupEntry(PGPS, AvatarTexture);
				PlayerContainer->AddChild(NewSlot);
				UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::UpdatePlayerEntry: Add PlayerEntry | Name: %s"), *PGPS->GetPlayerName());
			}
		}		
	}
}

void UPGFinalScoreBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ReturnToMainMenuButton)
	{
		ReturnToMainMenuButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked);
	}

	if (ReturnToLobbyButton)
	{
		ReturnToLobbyButton->OnClicked.AddDynamic(this, &UPGFinalScoreBoardWidget::OnReturnToLobbyButtonClicked);
	}
}

/*
* 메인 메뉴 버튼 -> 세션 종료 후 메인 메뉴로 이동
*/
void UPGFinalScoreBoardWidget::OnReturnToMainMenuButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("FinalScoreBoardWidget::ReturnToMainMenuButtonClicked: Clicked"));
	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
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
	if (PCRef)
	{
		if (APGPlayerController* PC = Cast<APGPlayerController>(PCRef))
		{
			PC->NotifyReadyToReturnLobby();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: PC Ref is not PG class"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FinalScoreBoardWidget::ReturnToLobbyButtonClicked: No PC Ref"));
	}
}
