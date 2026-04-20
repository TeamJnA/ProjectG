// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PGGameUserSettings.generated.h"

/**
 * Custom GameUserSettings that persists all user preferences.
 * Saved automatically to GameUserSettings.ini.
 *
 * SETUP: Add to DefaultEngine.ini:
 * [/Script/Engine.Engine]
 * GameUserSettingsClassName=/Script/ProjectG.PGGameUserSettings
 */
UCLASS()
class PROJECTG_API UPGGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UPGGameUserSettings();

	/** Static accessor */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UPGGameUserSettings* GetPGGameUserSettings();

	// -------- GamePlay --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|GamePlay")
	float CameraSensitivity;

	// -------- Audio Device --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Audio")
	FString OutputDeviceId;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Audio")
	FString InputDeviceId;

	// -------- Audio --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Audio")
	float MusicVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Audio")
	float SFXVolume;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Audio")
	float VoiceVolume;

	// -------- Mic --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Mic")
	float MicSensitivity;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|Mic")
	float MicInputGain;

	UPROPERTY(Config)
	bool bPushToTalk = false;

	void SetPushToTalk(bool bEnable);
	FORCEINLINE bool IsPushToTalk() const { return bPushToTalk; }

	// -------- Helper --------
	/** Apply mic CVar settings only */
	void ApplyMicSettings();	
};
