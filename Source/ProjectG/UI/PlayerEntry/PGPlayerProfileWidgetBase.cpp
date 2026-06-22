// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/PGPlayerProfileWidgetBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGPlayerState.h"
#include "Type/PGRankTypes.h"


bool UPGPlayerProfileWidgetBase::ResolveGameInstance()
{
	GIRef = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	return GIRef.IsValid();
}

void UPGPlayerProfileWidgetBase::ApplyAvatarAndName(APGPlayerState* LocalPlayerState)
{
	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	if (!GI || !LocalPlayerState)
	{
		return;
	}

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(LocalPlayerState->GetPlayerName()));
	}
	
	if (ProfileAvatar && LocalPlayerState->GetUniqueId().IsValid())
	{
		if (UTexture2D* Avatar = GI->GetSteamAvatarAsTexture(*LocalPlayerState->GetUniqueId().GetUniqueNetId()))
		{
			ProfileAvatar->SetBrushFromTexture(Avatar);
		}
	}
}

void UPGPlayerProfileWidgetBase::ApplyProgressForXP(int64 DisplayXP)
{
	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	if (!GI)
	{
		return;
	}

	const FPGRankProgress Prog = GI->GetRankProgressForXP(DisplayXP);
	if (RankNameText)
	{
		RankNameText->SetText(Prog.RankTitle);
	}

	float Percent = 1.0f;
	if (!Prog.bIsMaxRank && Prog.NextRankXP > Prog.CurrentRankFloorXP)
	{
		Percent = float(DisplayXP - Prog.CurrentRankFloorXP) / float(Prog.NextRankXP - Prog.CurrentRankFloorXP);
		Percent = FMath::Clamp(Percent, 0.0f, 1.0f);
	}

	if (XPBar)
	{
		XPBar->SetPercent(Percent);
	}

	if (XPText)
	{
		if (Prog.bIsMaxRank)
		{
			XPText->SetText(FText::FromString(TEXT("MAX")));
		}
		else
		{
			XPText->SetText(FText::FromString(FString::Printf(TEXT("%lld / %lld"),
				DisplayXP - Prog.CurrentRankFloorXP, Prog.NextRankXP - Prog.CurrentRankFloorXP)));
		}
	}

	if (Prog.RankIndex > LastShownRankIndex)
	{
		LastShownRankIndex = Prog.RankIndex;
		if (RankUpAnim)
		{
			PlayAnimation(RankUpAnim);
		}
	}
}
