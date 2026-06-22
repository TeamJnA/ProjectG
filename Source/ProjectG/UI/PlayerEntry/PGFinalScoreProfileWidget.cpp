// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/PGFinalScoreProfileWidget.h"
#include "Game/PGAdvancedFriendsGameInstance.h"


void UPGFinalScoreProfileWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GainTimerHandle);
	}

	Super::NativeDestruct();
}

void UPGFinalScoreProfileWidget::PlayResult(APGPlayerState* LocalPlayerState)
{
	if (!ResolveGameInstance())
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	ApplyAvatarAndName(LocalPlayerState);

	AnimStartXP = GI->GetPreMatchTotalXP();
	AnimTargetXP = AnimStartXP + GI->GetLastGainedXP();

	LastShownRankIndex = GI->GetRankProgressForXP(AnimStartXP).RankIndex;
	ApplyProgressForXP(AnimStartXP);

	StartGainAnimation();
}

void UPGFinalScoreProfileWidget::StartGainAnimation()
{
	if (AnimTargetXP <= AnimStartXP)
	{
		ApplyProgressForXP(AnimTargetXP);
		return;
	}

	AnimElapsed = 0.0f;
	AnimTotalDuration = FMath::Max(FillDuration, 0.01f);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(GainTimerHandle, this, &UPGFinalScoreProfileWidget::OnGainStep, GainStep, true);
	}
}

void UPGFinalScoreProfileWidget::OnGainStep()
{
	AnimElapsed += GainStep;
	const float Alpha = FMath::Clamp(AnimElapsed / AnimTotalDuration, 0.0f, 1.0f);
	const int64 DisplayXP = AnimStartXP + (int64)FMath::RoundToDouble((AnimTargetXP - AnimStartXP) * (double)Alpha);

	ApplyProgressForXP(DisplayXP);

	if (Alpha >= 1.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(GainTimerHandle);
		}
		ApplyProgressForXP(AnimTargetXP);
	}
}
