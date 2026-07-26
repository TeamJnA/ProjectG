// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/VoiceCapture.h"
#include "PGMicSettingWidget.generated.h"

class UComboBoxString;
class USlider;
class UProgressBar;
class UButton;
class UPGOptionSwitcherWidget;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMicSettingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetReturnToFocusWidget(UUserWidget* InWidget) { ReturnFocusWidget = InWidget; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	// -------- BindWidget --------
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> InputDeviceComboBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPGOptionSwitcherWidget> PushToTalkOption;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MicVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MicSensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> MicAmplitudeBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

private:
	// -------- Callbacks --------
	UFUNCTION()
	void OnInputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPushToTalkChanged(int32 OptionIndex);

	UFUNCTION()
	void OnMicVolumeChanged(float NewValue);

	UFUNCTION()
	void OnMicSensitivityChanged(float NewValue);

	UFUNCTION()
	void OnConfirmButtonClicked();

	// -------- Helpers --------
	void PopulateInputDevices();
	void LoadInitialSettings();
	void ApplyAndSaveSettings();

	float MicSensitivityToSlider(float Threshold) const;
	float SliderToMicSensitivity(float SliderValue) const;

	// -------- Variables --------
	TMap<FString, FString> InputDeviceNameToId;
	TSharedPtr<IVoiceCapture> SettingsVoiceCapture;
	TWeakObjectPtr<UUserWidget> ReturnFocusWidget;

	static constexpr float MicSensitivityMin = 0.001f;
	static constexpr float MicSensitivityMax = 0.02f;
	static constexpr float MicGainMax = 6.0f;

	float DisplayMicAmplitude = 0.0f;
	float AmplitudeUpdateAccumulator = 0.0f;

	static constexpr int32 VoiceDrainBufferSize = 8192;
	uint8 VoiceDrainBuffer[VoiceDrainBufferSize];

	bool bIsLoadingSettings = false;
};
