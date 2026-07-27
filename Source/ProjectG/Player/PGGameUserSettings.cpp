// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGGameUserSettings.h"


UPGGameUserSettings::UPGGameUserSettings()
	: CameraSensitivity(0.5f)
	, OutputDeviceId(FString())
	, InputDeviceId(FString())
	, MusicVolume(0.5f)
	, SFXVolume(0.5f)
	, VoiceVolume(0.5f)
	, MicSensitivity(0.01f)
	, MicInputGain(3.0f)
	, MicMode(EMicMode::OpenMic)
{
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
