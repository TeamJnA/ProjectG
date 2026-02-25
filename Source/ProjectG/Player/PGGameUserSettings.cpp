// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGGameUserSettings.h"

UPGGameUserSettings::UPGGameUserSettings()
	: CameraSensitivity(0.5f)
	, MusicVolume(0.5f)
	, SFXVolume(0.5f)
	, VoiceVolume(0.5f)
	, MicSensitivity(0.01f)
	, MicInputGain(3.0f)
{
}

UPGGameUserSettings* UPGGameUserSettings::GetPGGameUserSettings()
{
	return Cast<UPGGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UPGGameUserSettings::ApplyMicSettings()
{
	IConsoleManager& CM = IConsoleManager::Get();

	if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold")))
	{
		CVar->Set(MicSensitivity, ECVF_SetByGameSetting);
	}
	//if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.MicNoiseGateThreshold")))
	//{
	//	CVar->Set(MicSensitivity, ECVF_SetByGameSetting);
	//}
	if (IConsoleVariable* CVar = CM.FindConsoleVariable(TEXT("voice.MicInputGain")))
	{
		CVar->Set(MicInputGain, ECVF_SetByGameSetting);
	}
}
