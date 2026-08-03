// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "PGGameUserSettings.generated.h"


UENUM(BlueprintType)
enum class EMicMode : uint8
{
	OpenMic		UMETA(DisplayName = "OpenMic"),
	PushToTalk	UMETA(DisplayName = "Toggle"),
	Off			UMETA(DisplayName = "Off"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMicModeChanged, EMicMode, NewMicMode);

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
	EMicMode MicMode;

	UPROPERTY()
	FOnMicModeChanged OnMicModeChanged;

	void SetMicMode(EMicMode NewMode);
	FORCEINLINE EMicMode GetMicMode() const { return MicMode; }
	FORCEINLINE bool IsPushToTalk() const { return MicMode == EMicMode::PushToTalk; }
	FORCEINLINE bool IsMicOff() const { return MicMode == EMicMode::Off; }

	static EMicMode IndexToMicMode(int32 Index);
	static int32 MicModeToIndex(EMicMode Mode);

	// -------- Voice Setup --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|VoiceSetup")
	bool bHasCompletedInitialVoiceSetup = false;

	// -------- Helper --------
	/** Apply mic CVar settings only */
	void ApplyMicSettings();
};
