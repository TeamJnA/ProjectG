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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMicToggleChanged, bool, bMicToggled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInitialVoiceSetupComplete);

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

	bool IsMicReady() const;

	/** Static accessor */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static UPGGameUserSettings* GetPGGameUserSettings();

	// -------- GamePlay --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|GamePlay")
	float CameraSensitivity;

	UPROPERTY(Config)
	FString LanguageCulture;

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
	bool bMicToggleActive;

	UPROPERTY()
	FOnMicModeChanged OnMicModeChanged;

	UPROPERTY()
	FOnMicToggleChanged OnMicToggleChanged;

	void SetMicMode(EMicMode NewMode);
	FORCEINLINE EMicMode GetMicMode() const { return MicMode; }
	FORCEINLINE bool IsPushToTalk() const { return MicMode == EMicMode::PushToTalk; }
	FORCEINLINE bool IsMicOff() const { return MicMode == EMicMode::Off; }

	FORCEINLINE bool IsMicToggled() const { return bMicToggleActive; }
	void SetMicToggle(bool bToggleActive);

	static EMicMode IndexToMicMode(int32 Index);
	static int32 MicModeToIndex(EMicMode Mode);

	// -------- Voice Setup --------
	UPROPERTY(Config, BlueprintReadWrite, Category = "Settings|VoiceSetup")
	bool bHasCompletedInitialVoiceSetup = false;

	void SetInitialVoiceSetupCompleted();

	UPROPERTY(BlueprintAssignable)
	FOnInitialVoiceSetupComplete OnInitialVoiceSetupComplete;

	// -------- Video --------
	/*
	* Resolution, Anti-Aliasing, GlobalIllumination 세 가지를 설정
	*/
	FORCEINLINE int32 GetOverallVideoQualityLevel() const { return OverallVideoQualityLevel; }
	void SetAndApplyOverallVideoQuality(int32 Value);

	UPROPERTY(Config)
	int32 OverallVideoQualityLevel;


	// -------- Helper --------
	/** Apply mic CVar settings only */
	void ApplyMicSettings();
};
