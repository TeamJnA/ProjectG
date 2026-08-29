// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGGameUserSettings.h"


UPGGameUserSettings::UPGGameUserSettings()
	: CameraSensitivity(0.5f)
	, LanguageCulture(TEXT("en"))
	, OutputDeviceId(FString())
	, InputDeviceId(FString())
	, MusicVolume(0.5f)
	, SFXVolume(0.5f)
	, VoiceVolume(0.5f)
	, MicSensitivity(0.01f)
	, MicInputGain(3.0f)
	, MicMode(EMicMode::OpenMic)
	, bMicToggleActive(false)
	, OverallVideoQualityLevel(2)
{
}

bool UPGGameUserSettings::IsMicReady() const
{
	switch (GetMicMode())
	{
	case EMicMode::OpenMic:
		return true;

	case EMicMode::Off:
		return false;

	case EMicMode::PushToTalk:
	{
		if (IsMicToggled())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	break;
	}

	return false;
}

UPGGameUserSettings* UPGGameUserSettings::GetPGGameUserSettings()
{
	return Cast<UPGGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UPGGameUserSettings::SetMicMode(EMicMode NewMode)
{
	if (MicMode == NewMode)
	{
		return;
	}

	MicMode = NewMode;

	SaveSettings();
	SaveConfig();

	OnMicModeChanged.Broadcast(NewMode);

	if (bMicToggleActive)
	{
		bMicToggleActive = false;
		OnMicToggleChanged.Broadcast(false);
	}
}

void UPGGameUserSettings::SetMicToggle(bool bToggleActive)
{
	if (bMicToggleActive == bToggleActive)
	{
		return;
	}

	bMicToggleActive = bToggleActive;
	OnMicToggleChanged.Broadcast(bMicToggleActive);
}

EMicMode UPGGameUserSettings::IndexToMicMode(int32 Index)
{
	switch (Index)
	{
	case 1: 
		return EMicMode::PushToTalk;

	case 2: 
		return EMicMode::Off;

	default:
		return EMicMode::OpenMic;
	}
}

int32 UPGGameUserSettings::MicModeToIndex(EMicMode Mode)
{
	switch (Mode)
	{
	case EMicMode::PushToTalk:
		return 1;

	case EMicMode::Off:
		return 2;

	default:
		return 0;
	}
}

void UPGGameUserSettings::SetInitialVoiceSetupCompleted()
{
	bHasCompletedInitialVoiceSetup = true;
	OnInitialVoiceSetupComplete.Broadcast();
}

void UPGGameUserSettings::SetAndApplyOverallVideoQuality(int32 Value)
{
	check(Value >= 0 && Value <= 2);

	OverallVideoQualityLevel = Value;

	// Value(0, 1, 2)에 따른 설정값 매핑 배열
	const float ResolutionScales[3] = { 75.0f, 85.0f, 100.0f };
	const int32 AAMethods[3] = { 1, 4, 4 }; // 1: FXAA, 2: TAA, 4: TSR
	const int32 GIQualities[3] = { 1, 2, 3 };

	// Resolution (해상도 스케일) 설정
	SetResolutionScaleValueEx(ResolutionScales[Value]);

	// 안티앨리어싱 종류 (FXAA / TAA / TSR) 설정
	if (IConsoleVariable* AAMethodCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod")))
	{
		AAMethodCVar->Set(AAMethods[Value], ECVF_SetByGameSetting);
	}

	// Global Illumination (글로벌 일루미네이션) 퀄리티 설정
	SetGlobalIlluminationQuality(GIQualities[Value]);
}

void UPGGameUserSettings::ApplyMicSettings()
{
	IConsoleManager& CM = IConsoleManager::Get();

	if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold")))
	{
		CVar->Set(MicSensitivity, ECVF_SetByConsole);
	}

	if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold")))
	{
		CVar->Set(MicSensitivity, ECVF_SetByConsole);
	}

	if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.MicInputGain")))
	{
		CVar->Set(MicInputGain, ECVF_SetByConsole);
	}
}
