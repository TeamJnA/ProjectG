// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/PGMainMenuProfileWidget.h"
#include "Game/PGAdvancedFriendsGameInstance.h"


void UPGMainMenuProfileWidget::SetupStatic(APGPlayerState* LocalPlayerState)
{	
	if (!ResolveGameInstance())
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	ApplyAvatarAndName(LocalPlayerState);

	const int64 Total = GI->GetTotalXP();
	LastShownRankIndex = GI->GetRankProgressForXP(Total).RankIndex;
	ApplyProgressForXP(Total);
}
