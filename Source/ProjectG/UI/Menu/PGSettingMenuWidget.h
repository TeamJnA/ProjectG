// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PGGameUserSettings.h"
#include "Interfaces/VoiceCapture.h"

#include "PGSettingMenuWidget.generated.h"

class UVerticalBox;
class USlider;
/* TODO 프로그레스 바 제거 필요(헤더도)*/
class UProgressBar;
class UImage;
class UButton;
class UComboBoxString;
class UWidgetSwitcher;
class UPGPlayerVoiceEntry;
class UPGOptionSwitcherWidget;
class UPGAdvancedFriendsGameInstance;
class APGGameState;
class APGPlayerController;
class APGLobbyPlayerController;
class UPGGameUserSettings;

/**
 * Settings menu widget handling GamePlay, Device, Audio, Mic, and Video settings.
 */
UCLASS()
class PROJECTG_API UPGSettingMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void ActivateMicCapture();
    void DeactivateMicCapture();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

    void UpdateMicTestBar(float InDeltaTime);

    UFUNCTION()
    void OnMicModeChanged(EMicMode InMicMode);

    UFUNCTION()
    void OnMicToggleChanged(bool bMicToggled);

    void SetMicIcon();

    // -------- Player Voice List --------
    UFUNCTION()
    void UpdatePlayerVoiceList();

    // -------- GamePlay --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> CameraSensitivitySlider;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> LanguageComboBox;

    // -------- Audio Device --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> OutputDeviceComboBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> InputDeviceComboBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> PushToTalkOption;

    // -------- Audio --------
    // UPROPERTY(meta = (BindWidget))
    // TObjectPtr<USlider> MusicVolumeSlider;

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
    TObjectPtr<UImage> MicTestBar;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> MicTestBarMID;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MicModeIcon;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> MicStopSlash;

    UPROPERTY(EditDefaultsOnly, Category = "Indicator")
    FName FillRatioParam = TEXT("FillRatio");

    TWeakObjectPtr<UPGGameUserSettings> CachedSettings;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> MicVolumeSlider;

    // -------- Video --------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPGOptionSwitcherWidget> OverallGraphicsOption;

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
    // -------- GamePlay Callbacks --------
    UFUNCTION()
    void OnCameraSensitivityChanged(float NewValue);

    UFUNCTION()
    void OnLanguageSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    void PopulateLanguages();

    // -------- Audio Callbacks --------
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
    /*
                    상 중 하
    Resolution 60 80 100
    안티앨리어싱 종류 1(FXAA) 2(TAA) 4(TSR)
    GlobalIllumination Quality Option 1 2 3
    */
    UFUNCTION()
    void OnOverallGraphicsChanged(int32 OptionIndex);

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

    UFUNCTION()
    void OnPushToTalkChanged(int32 OptionIndex);

    // -------- Helpers --------
    /** Load saved settings and apply to sliders */
    void LoadAndApplySettings();

    /** Apply and Save current settings to PGGameUserSettings */ 
    void ApplyAndSaveSettings();

    /** Convert mic sensitivity CVar value to normalized slider value (0~1) */
    float MicSensitivityToSlider(float Threshold) const;

    /** Convert normalized slider value (0~1) to mic sensitivity CVar value */
    float SliderToMicSensitivity(float SliderValue) const;

    // -------- Variables --------
    // Device ComboBox 표시명 -> DeviceId 매핑
    TMap<FString, FString> OutputDeviceNameToId;
    TMap<FString, FString> InputDeviceNameToId;
    TMap<FString, FString> LanguageNameToCulture;

    TWeakObjectPtr<UPGAdvancedFriendsGameInstance> GIRef;
    TWeakObjectPtr<APGGameState> GSRef;

    TSharedPtr<IVoiceCapture> SettingsVoiceCapture;

    static constexpr float MicSensitivityMin = 0.001f;
    static constexpr float MicSensitivityMax = 0.02f;
    static constexpr float MicGainMax = 6.0f;
    float DisplayMicAmplitude = 0.0f;
    float AmplitudeUpdateAccumulator = 0.0f;
    
    static constexpr int32 VoiceDrainBufferSize = 8192;
    uint8 VoiceDrainBuffer[VoiceDrainBufferSize];

    bool bIsLoadingSettings = false;    
};
