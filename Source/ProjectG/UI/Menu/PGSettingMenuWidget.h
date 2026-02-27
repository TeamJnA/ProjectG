// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGSettingMenuWidget.generated.h"

class UVerticalBox;
class USlider;
class UButton;
class UComboBoxString;
class UWidgetSwitcher;
class UPGPlayerVoiceEntry;
class UPGOptionSwitcherWidget;
class UPGAdvancedFriendsGameInstance;
class APGGameState;
class UPGGameUserSettings;

/**
 * Settings menu widget handling GamePlay, Device, Audio, Mic, and Video settings.
 */
UCLASS()
class PROJECTG_API UPGSettingMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // -------- Player Voice List --------
    UFUNCTION()
    void UpdatePlayerVoiceList();

    // -------- GamePlay --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> CameraSensitivitySlider;

    // -------- Audio Device --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> OutputDeviceComboBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> InputDeviceComboBox;

    // -------- Audio --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MusicVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> SFXVolumeSlider;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> VoiceVolumeSlider;

    // -------- Player Voice List --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> PlayerVoiceListContainer;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPGPlayerVoiceEntry> PlayerVoiceEntryWidgetClass;

    // -------- Mic --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MicSensitivitySlider;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MicVolumeSlider;

    // -------- Video --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> OverallGraphicsOption;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> TextureQualityOption;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> ShadowQualityOption;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> ViewDistanceQualityOption;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> AntiAliasingQualityOption;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> VSyncOption;

    // -------- Widget Switcher --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> GamePlayOptionButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> AudioOptionButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> VideoOptionButton;

private:
    // -------- Slider Callbacks --------
    UFUNCTION()
    void OnCameraSensitivityChanged(float NewValue);

    UFUNCTION()
    void OnMusicVolumeChanged(float NewValue);

    UFUNCTION()
    void OnSFXVolumeChanged(float NewValue);

    UFUNCTION()
    void OnVoiceVolumeChanged(float NewValue);

    UFUNCTION()
    void OnMicSensitivityChanged(float NewValue);

    UFUNCTION()
    void OnMicVolumeChanged(float NewValue);

    // -------- Video Callbacks --------
    UFUNCTION()
    void OnOverallGraphicsChanged(int32 OptionIndex);

    UFUNCTION()
    void OnTextureQualityChanged(int32 OptionIndex);

    UFUNCTION()
    void OnShadowQualityChanged(int32 OptionIndex);

    UFUNCTION()
    void OnViewDistanceQualityChanged(int32 OptionIndex);

    UFUNCTION()
    void OnAntiAliasingQualityChanged(int32 OptionIndex);

    UFUNCTION()
    void OnVSyncChanged(int32 OptionIndex);

    // -------- WidgetSwitch Callbacks --------
    UFUNCTION()
    void OnGamePlayOptionButtonClicked();

    UFUNCTION()
    void OnAudioOptionButtonClicked();

    UFUNCTION()
    void OnVideoOptionButtonClicked();

    // -------- Device Callbacks --------
    UFUNCTION()
    void OnOutputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnInputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    void EnumerateAudioDevices();

    UFUNCTION()
    void OnOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);

    void PopulateInputDevices();

    UFUNCTION()
    void OnOutputDeviceSwapComplete(const FSwapAudioOutputResult& SwapResult);

    // -------- Helpers --------
    /** Load saved settings and apply to sliders */
    void LoadAndApplySettings();

    /** Apply and Save current settings to PGGameUserSettings */ 
    void ApplyAndSaveSettings();

    /** After OverallGraphics changes, update individual quality widgets to reflect new values */
    void RefreshIndividualQualityWidgets();

    /** Update OverallGraphics indicator after individual quality change */
    void UpdateOverallGraphicsIndicator();

    /** Convert mic sensitivity CVar value to normalized slider value (0~1) */
    float MicSensitivityToSlider(float Threshold) const;

    /** Convert normalized slider value (0~1) to mic sensitivity CVar value */
    float SliderToMicSensitivity(float SliderValue) const;

    // -------- Variables --------
    // Device ComboBox 표시명 -> DeviceId 매핑
    TMap<FString, FString> OutputDeviceNameToId;
    TMap<FString, FString> InputDeviceNameToId;

    TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
    TWeakObjectPtr<APGGameState> GSRef;

    static constexpr float MicSensitivityMin = 0.001f;
    static constexpr float MicSensitivityMax = 0.02f;
    static constexpr float MicGainMax = 6.0f;

    bool bIsLoadingSettings = false;
};
