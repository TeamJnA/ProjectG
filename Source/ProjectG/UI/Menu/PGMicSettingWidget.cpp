// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGMicSettingWidget.h"
#include "UI/Menu/PGOptionSwitcherWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"

#include "Player/PGGameUserSettings.h"
#include "Utils/PGVoiceUtils.h"
#include "VoiceModule.h"
#include "AudioCapture.h"


void UPGMicSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();

	if (InputDeviceComboBox)
	{
		InputDeviceComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPGMicSettingWidget::OnInputDeviceSelectionChanged);
	}

	if (PushToTalkOption)
	{
		PushToTalkOption->OnOptionChanged.AddUniqueDynamic(this, &UPGMicSettingWidget::OnPushToTalkChanged);
	}
	
	if (MicVolumeSlider)
	{
		MicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGMicSettingWidget::OnMicVolumeChanged);
	}

	if (MicSensitivitySlider)
	{
		MicSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UPGMicSettingWidget::OnMicSensitivityChanged);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(this, &UPGMicSettingWidget::OnConfirmButtonClicked);
	}

	// 디바이스 리스트 채우기 / 초기값 로드
	PopulateInputDevices();
	LoadInitialSettings();

	// 마이크 캡처 시작
	FString DeviceName = InputDeviceComboBox ? InputDeviceComboBox->GetSelectedOption() : TEXT("");
	SettingsVoiceCapture = FVoiceModule::Get().CreateVoiceCapture(DeviceName);
	if (SettingsVoiceCapture.IsValid())
	{
		SettingsVoiceCapture->Start();
	}
}

void UPGMicSettingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!SettingsVoiceCapture.IsValid() || !MicAmplitudeBar)
	{
		return;
	}

	AmplitudeUpdateAccumulator += InDeltaTime;
	if (AmplitudeUpdateAccumulator < 0.033f)
	{
		return;
	}
	const float EffectiveDelta = AmplitudeUpdateAccumulator;
	AmplitudeUpdateAccumulator = 0.0f;

	uint32 AvailableData = 0;
	if (SettingsVoiceCapture->GetCaptureState(AvailableData) == EVoiceCaptureState::Ok && AvailableData > 0)
	{
		uint32 ReadData = 0;
		SettingsVoiceCapture->GetVoiceData(VoiceDrainBuffer, VoiceDrainBufferSize, ReadData);
	}

	float RawAmplitude = SettingsVoiceCapture->GetCurrentAmplitude();
	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		if (Settings->IsMicOff())
		{
			RawAmplitude = 0.0f;
		}

		if (RawAmplitude < Settings->MicSensitivity)
		{
			RawAmplitude = 0.0f;
			DisplayMicAmplitude = 0.0f;
		}
		else
		{
			RawAmplitude *= (Settings->MicInputGain / 3.0f);
		}
	}

	const float MaxAmplitude = 0.3f;
	RawAmplitude = FMath::Min(RawAmplitude, MaxAmplitude);

	const float Speed = (RawAmplitude > DisplayMicAmplitude) ? 9.0f : 5.0f;
	DisplayMicAmplitude = FMath::FInterpTo(DisplayMicAmplitude, RawAmplitude, EffectiveDelta, Speed);

	MicAmplitudeBar->SetPercent(FMath::Clamp(DisplayMicAmplitude / MaxAmplitude, 0.0f, 1.0f));
}

void UPGMicSettingWidget::NativeDestruct()
{
	if (SettingsVoiceCapture.IsValid())
	{
		SettingsVoiceCapture->Stop();
		SettingsVoiceCapture.Reset();
	}

	Super::NativeDestruct();
}

void UPGMicSettingWidget::PopulateInputDevices()
{
	if (!InputDeviceComboBox)
	{
		return;
	}

	bIsLoadingSettings = true;
	InputDeviceComboBox->ClearOptions();
	InputDeviceNameToId.Empty();

	Audio::FAudioCapture AudioCapture;
	TArray<Audio::FCaptureDeviceInfo> Devices;
	AudioCapture.GetCaptureDevicesAvailable(Devices);

	UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
	FString SavedDeviceId = Settings ? Settings->InputDeviceId : FString();
	FString DefaultName, SelectedName;

	for (const Audio::FCaptureDeviceInfo& Device : Devices)
	{
		InputDeviceNameToId.Add(Device.DeviceName, Device.DeviceId);
		InputDeviceComboBox->AddOption(Device.DeviceName);

		if (DefaultName.IsEmpty())
		{
			DefaultName = Device.DeviceName;
		}

		if (Device.DeviceId == SavedDeviceId)
		{
			SelectedName = Device.DeviceName;
		}
	}

	InputDeviceComboBox->SetSelectedOption(SelectedName.IsEmpty() ? DefaultName : SelectedName);
	bIsLoadingSettings = false;
}

void UPGMicSettingWidget::LoadInitialSettings()
{
	UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
	if (!Settings)
	{
		return;
	}

	bIsLoadingSettings = true;

	if (PushToTalkOption)
	{
		PushToTalkOption->SetSelectedIndex(UPGGameUserSettings::MicModeToIndex(Settings->GetMicMode()), false);
	}

	if (MicSensitivitySlider)
	{
		MicSensitivitySlider->SetValue(MicSensitivityToSlider(Settings->MicSensitivity));
	}

	if (MicVolumeSlider)
	{
		MicVolumeSlider->SetValue(Settings->MicInputGain / MicGainMax);
	}

	bIsLoadingSettings = false;
}

void UPGMicSettingWidget::OnInputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bIsLoadingSettings)
	{
		return;
	}

	FString* DeviceId = InputDeviceNameToId.Find(SelectedItem);
	if (!DeviceId)
	{
		return;
	}

	PGVoiceUtils::ChangeInputDevice(GetWorld(), SelectedItem);

	if (SettingsVoiceCapture.IsValid())
	{
		SettingsVoiceCapture->Stop();
		SettingsVoiceCapture = FVoiceModule::Get().CreateVoiceCapture(SelectedItem);
		if (SettingsVoiceCapture.IsValid())
		{
			SettingsVoiceCapture->Start();
		}
	}

	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->InputDeviceId = *DeviceId;
		ApplyAndSaveSettings();
	}
}

void UPGMicSettingWidget::OnPushToTalkChanged(int32 OptionIndex)
{
	if (bIsLoadingSettings)
	{
		return;
	}

	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->SetMicMode(UPGGameUserSettings::IndexToMicMode(OptionIndex));
		ApplyAndSaveSettings();
	}
}

void UPGMicSettingWidget::OnMicVolumeChanged(float NewValue)
{
	if (bIsLoadingSettings)
	{
		return;
	}

	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->MicInputGain = NewValue * MicGainMax;
		Settings->ApplyMicSettings();
		ApplyAndSaveSettings();
	}
}

void UPGMicSettingWidget::OnMicSensitivityChanged(float NewValue)
{
	if (bIsLoadingSettings)
	{
		return;
	}

	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->MicSensitivity = SliderToMicSensitivity(NewValue);
		Settings->ApplyMicSettings();
		ApplyAndSaveSettings();
	}
}

void UPGMicSettingWidget::OnConfirmButtonClicked()
{
	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->bHasCompletedInitialVoiceSetup = true;
		ApplyAndSaveSettings();
	}

	if (UUserWidget* ParentWidget = ReturnFocusWidget.Get())
	{
		ParentWidget->SetKeyboardFocus();
	}

	RemoveFromParent();
}

void UPGMicSettingWidget::ApplyAndSaveSettings()
{
	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		Settings->ApplySettings(true);
		Settings->SaveSettings();
	}
}

float UPGMicSettingWidget::MicSensitivityToSlider(float Threshold) const
{
	const float Clamped = FMath::Clamp(Threshold, MicSensitivityMin, MicSensitivityMax);
	return (1.0f - FMath::GetRangePct(MicSensitivityMin, MicSensitivityMax, Clamped));
}

float UPGMicSettingWidget::SliderToMicSensitivity(float SliderValue) const
{
	return FMath::Lerp(MicSensitivityMax, MicSensitivityMin, SliderValue);
}
