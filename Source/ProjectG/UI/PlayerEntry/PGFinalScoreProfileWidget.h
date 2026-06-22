// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PlayerEntry/PGPlayerProfileWidgetBase.h"
#include "PGFinalScoreProfileWidget.generated.h"

class APGPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGFinalScoreProfileWidget : public UPGPlayerProfileWidgetBase
{
	GENERATED_BODY()

public:
	void PlayResult(APGPlayerState* LocalPlayerState);

protected:
	virtual void NativeDestruct() override;

	void StartGainAnimation();
	void OnGainStep();

	FTimerHandle GainTimerHandle;

	int64 AnimStartXP = 0;
	int64 AnimTargetXP = 0;

	UPROPERTY(EditDefaultsOnly, Category = "XP")
	float FillDuration = 1.5f;

	float AnimElapsed = 0.0f;
	float AnimTotalDuration = 0.0f;

	static constexpr float GainStep = 0.02f;	
};
