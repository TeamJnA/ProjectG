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
