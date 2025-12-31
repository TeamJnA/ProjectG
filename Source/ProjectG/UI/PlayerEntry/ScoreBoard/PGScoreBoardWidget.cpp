// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/ScoreBoard/PGScoreBoardWidget.h"

#include "UI/PlayerEntry/PGPlayerEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGPlayerController.h"
#include "Character/PGPlayerCharacter.h"


void UPGScoreBoardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SpectateButton)
	{
		SpectateButton->OnClicked.AddDynamic(this, &UPGScoreBoardWidget::OnSpectateButtonClicked);
	}

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;
	}

	if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
	{
		GSRef = GS;
		GS->OnPlayerArrayChanged.RemoveAll(this);
		GS->OnPlayerArrayChanged.AddDynamic(this, &UPGScoreBoardWidget::UpdatePlayerEntry);

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
			{
				PGPS->OnPlayerStateUpdated.RemoveAll(this);
				PGPS->OnPlayerStateUpdated.AddDynamic(this, &UPGScoreBoardWidget::UpdatePlayerEntry);
			}
		}
	}
}

void UPGScoreBoardWidget::NativeDestruct()
{
	if (APGGameState* GS = GSRef.Get())
	{
		GS->OnPlayerArrayChanged.RemoveAll(this);

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (!IsValid(PS))
			{
				continue;
			}

			if (APGPlayerState* PGPS = Cast<APGPlayerState>(PS))
			{
				PGPS->OnPlayerStateUpdated.RemoveAll(this);
			}
		}
	}

	Super::NativeDestruct();
}

/*
* 로컬 플레이어에 ScoreBoardWidget 생성 이후 플레이어 목록 업데이트
* GameState의 PlayerList 업데이트 시 전달되는 OnPlayerListUpdated 델리게이트에 업데이트 함수 바인드
*/
void UPGScoreBoardWidget::BindPlayerEntry()
{
	UE_LOG(LogTemp, Log, TEXT("UPGScoreBoardWidget::BindPlayerEntry: InPlayerCharacter is valid. Binding delegate."));

	UpdatePlayerEntry();
}

/*
* 플레이어 목록 업데이트
* 플레이어들의 Steam 프로필 이미지, 이름, 종료 상태 디스플레이
*/
void UPGScoreBoardWidget::UpdatePlayerEntry()
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
* 관전 버튼 클릭 시 관전 모드 진입
*/
void UPGScoreBoardWidget::OnSpectateButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("ScoreBoardWidget::OnSpectateButtonClicked: Spectate button clicked"), *GetOwningPlayer()->GetName());
	
	APGPlayerController* PC = Cast<APGPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	if (PC->IsLocalController())
	{
		PC->StartSpectate();
	}
}
