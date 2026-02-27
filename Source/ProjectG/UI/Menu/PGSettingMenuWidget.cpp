// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGSettingMenuWidget.h"
#include "UI/PlayerEntry/PGPlayerVoiceEntry.h"
#include "UI/Menu/PGOptionSwitcherWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ComboBoxString.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Player/PGGameUserSettings.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "AudioMixerBlueprintLibrary.h"
#include "AudioCapture.h"
#include "Utils/PGVoiceUtils.h"


void UPGSettingMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
    {
        GIRef = GI;
    }

    // -------- Bind Gameplay Option Callbacks --------
    if (CameraSensitivitySlider)
    {
        CameraSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnCameraSensitivityChanged);
    }

    // -------- Bind Device Option Callbacks --------
    if (OutputDeviceComboBox)
    {
        OutputDeviceComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnOutputDeviceSelectionChanged);
    }

    if (InputDeviceComboBox)
    {
        InputDeviceComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnInputDeviceSelectionChanged);
    }

    // -------- Bind Audio Option Callbacks --------
    if (MusicVolumeSlider)
    {
        MusicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnMusicVolumeChanged);
    }

    if (SFXVolumeSlider)
    {
        SFXVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnSFXVolumeChanged);
    }

    if (VoiceVolumeSlider)
    {
        VoiceVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnVoiceVolumeChanged);
    }

    // Mic Sensitivity (SilenceDetectionThreshold: 0.001 ~ 0.02)
    if (MicSensitivitySlider)
    {
        MicSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnMicSensitivityChanged);
    }

    // Mic Volume (MicInputGain: 0.0 ~ 6.0)
    if (MicVolumeSlider)
    {
        MicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnMicVolumeChanged);
    }

    // -------- Bind Video Option Callbacks --------
    if (OverallGraphicsOption)
    {
        OverallGraphicsOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnOverallGraphicsChanged);
    }

    if (TextureQualityOption)
    {
        TextureQualityOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnTextureQualityChanged);
    }

    if (ShadowQualityOption)
    {
        ShadowQualityOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnShadowQualityChanged);
    }

    if (ViewDistanceQualityOption)
    {
        ViewDistanceQualityOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnViewDistanceQualityChanged);
    }

    if (AntiAliasingQualityOption)
    {
        AntiAliasingQualityOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnAntiAliasingQualityChanged);
    }

    if (VSyncOption)
    {
        VSyncOption->OnOptionChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnVSyncChanged);
    }

    if (GamePlayOptionButton)
    {
        GamePlayOptionButton->OnClicked.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnGamePlayOptionButtonClicked);
    }

    if (AudioOptionButton)
    {
        AudioOptionButton->OnClicked.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnAudioOptionButtonClicked);
    }

    if (VideoOptionButton)
    {
        VideoOptionButton->OnClicked.AddUniqueDynamic(this, &UPGSettingMenuWidget::OnVideoOptionButtonClicked);
    }
}

void UPGSettingMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APGGameState* GS = GetWorld()->GetGameState<APGGameState>())
    {
        GSRef = GS;
        GS->OnPlayerArrayChanged.AddUniqueDynamic(this, &UPGSettingMenuWidget::UpdatePlayerVoiceList);
    }

    UE_LOG(LogTemp, Log, TEXT("[SettingMenu] Update setting menu"));
    LoadAndApplySettings();
    UpdatePlayerVoiceList();
}

void UPGSettingMenuWidget::NativeDestruct()
{
    if (APGGameState* GS = GSRef.Get())
    {
        GS->OnPlayerArrayChanged.RemoveAll(this);
    }

    Super::NativeDestruct();
}

/**
 * Update voice player list based on GameState's PlayerArray
 * Create voice entry widget for each player except local player
 */
void UPGSettingMenuWidget::UpdatePlayerVoiceList()
{
    if (!PlayerVoiceListContainer || !PlayerVoiceEntryWidgetClass)
    {
        return;
    }

    APGGameState* GS = GSRef.Get();
    if (!GS)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[SettingMenu] UpdatePlayerVoiceList: Updating voice player list"));

    // Clear existing widgets
    PlayerVoiceListContainer->ClearChildren();

    APlayerController* LocalPC = GetOwningPlayer();
    APlayerState* LocalPlayerState = LocalPC ? LocalPC->PlayerState : nullptr;

    // Create voice entry widget for each player except local player
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS)
        {
            continue;
        }

        // Skip local player
        if (PS == LocalPlayerState)
        {
            continue;
        }

        // Create voice player entry widget
        UPGPlayerVoiceEntry* VoiceEntryWidget = CreateWidget<UPGPlayerVoiceEntry>(this, PlayerVoiceEntryWidgetClass);
        if (VoiceEntryWidget)
        {
            VoiceEntryWidget->SetupEntry(PS);
            PlayerVoiceListContainer->AddChild(VoiceEntryWidget);

            UE_LOG(LogTemp, Log, TEXT("[SettingMenu] Created voice entry for player: %s"), *PS->GetPlayerName());
        }
    }
}

// Load Settings and apply to sliders
void UPGSettingMenuWidget::LoadAndApplySettings()
{
    UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
    if (!Settings)
    {
        return;
    }

    bIsLoadingSettings = true;

    // GamePlay
    if (CameraSensitivitySlider)
    {
        CameraSensitivitySlider->SetValue(Settings->CameraSensitivity);
    }

    // Audio - set sliders and apply SoundMix overrides
    if (MusicVolumeSlider)
    {
        MusicVolumeSlider->SetValue(Settings->MusicVolume);
    }

    if (SFXVolumeSlider)
    {
        SFXVolumeSlider->SetValue(Settings->SFXVolume);
    }

    if (VoiceVolumeSlider)
    {
        VoiceVolumeSlider->SetValue(Settings->VoiceVolume);
    }

    // Mic
    if (MicSensitivitySlider)
    {
        MicSensitivitySlider->SetValue(MicSensitivityToSlider(Settings->MicSensitivity));
    }

    if (MicVolumeSlider)
    {
        MicVolumeSlider->SetValue(Settings->MicInputGain / MicGainMax);
    }

    bIsLoadingSettings = false;

    // Apply mic CVars
    Settings->ApplyMicSettings();

    // Video
    // Overall quality: 0=Low, 1=Medium, 2=High, 3=Ultra, 4(-1) = Custom
    if (OverallGraphicsOption)
    {
        int32 OverallLevel = Settings->GetOverallScalabilityLevel();
        if (OverallLevel >= 0)
        {
            OverallGraphicsOption->SetSelectedIndex(OverallLevel, false);
        }
        else
        {
            OverallGraphicsOption->SetSelectedIndex(4, false);
        }
    }

    // Refresh each video options
    RefreshIndividualQualityWidgets();

    // VSync: 0=OFF, 1=ON
    if (VSyncOption)
    {
        VSyncOption->SetSelectedIndex(Settings->IsVSyncEnabled() ? 1 : 0, false);
    }

    // Find/Set available devices and Load saved device settings
    EnumerateAudioDevices();
}

// -------- Gameplay --------
void UPGSettingMenuWidget::OnCameraSensitivityChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->CameraSensitivity = NewValue;
        ApplyAndSaveSettings();
    }
}

// -------- Audio --------
void UPGSettingMenuWidget::OnMusicVolumeChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
    {
        GI->ApplySoundMixOverride(GetWorld(), GI->GetSoundClass_Music(), NewValue);
    }

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->MusicVolume = NewValue;
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::OnSFXVolumeChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
    {
        GI->ApplySoundMixOverride(GetWorld(), GI->GetSoundClass_SFX(), NewValue);
    }

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SFXVolume = NewValue;
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::OnVoiceVolumeChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
    {
        GI->ApplySoundMixOverride(GetWorld(), GI->GetSoundClass_Voice(), NewValue);
    }

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->VoiceVolume = NewValue;
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::OnMicSensitivityChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    const float Threshold = SliderToMicSensitivity(NewValue);
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->MicSensitivity = Threshold;
        Settings->ApplyMicSettings();
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::OnMicVolumeChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    const float Gain = NewValue * MicGainMax;
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->MicInputGain = Gain;
        Settings->ApplyMicSettings();
        ApplyAndSaveSettings();
    }
}

// -------- Video --------
void UPGSettingMenuWidget::OnOverallGraphicsChanged(int32 OptionIndex)
{
    UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("no gamesetting"));
        return;
    }

    // SetOverallScalabilityLevel sets all individual qualities at once
    Settings->SetOverallScalabilityLevel(OptionIndex);
    ApplyAndSaveSettings();

    // Update individual switchers to reflect the overall change
    RefreshIndividualQualityWidgets();
}

void UPGSettingMenuWidget::RefreshIndividualQualityWidgets()
{
    UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
    if (!Settings)
    {
        return;
    }

    if (TextureQualityOption)
    {
        TextureQualityOption->SetSelectedIndex(Settings->GetTextureQuality(), false);
    }

    if (ShadowQualityOption)
    {
        ShadowQualityOption->SetSelectedIndex(Settings->GetShadowQuality(), false);
    }

    if (ViewDistanceQualityOption)
    {
        ViewDistanceQualityOption->SetSelectedIndex(Settings->GetViewDistanceQuality(), false);
    }

    if (AntiAliasingQualityOption)
    {
        AntiAliasingQualityOption->SetSelectedIndex(Settings->GetAntiAliasingQuality(), false);
    }
}

void UPGSettingMenuWidget::OnTextureQualityChanged(int32 OptionIndex)
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SetTextureQuality(OptionIndex);
        ApplyAndSaveSettings();
        UpdateOverallGraphicsIndicator();
    }
}

void UPGSettingMenuWidget::OnShadowQualityChanged(int32 OptionIndex)
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SetShadowQuality(OptionIndex);
        ApplyAndSaveSettings();
        UpdateOverallGraphicsIndicator();
    }
}

void UPGSettingMenuWidget::OnViewDistanceQualityChanged(int32 OptionIndex)
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SetViewDistanceQuality(OptionIndex);
        ApplyAndSaveSettings();
        UpdateOverallGraphicsIndicator();
    }
}

void UPGSettingMenuWidget::OnAntiAliasingQualityChanged(int32 OptionIndex)
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SetAntiAliasingQuality(OptionIndex);
        ApplyAndSaveSettings();
        UpdateOverallGraphicsIndicator();
    }
}

void UPGSettingMenuWidget::OnVSyncChanged(int32 OptionIndex)
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->SetVSyncEnabled(OptionIndex == 1);
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::UpdateOverallGraphicsIndicator()
{
    if (!OverallGraphicsOption)
    {
        return;
    }

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        int32 OverallLevel = Settings->GetOverallScalabilityLevel();
        OverallGraphicsOption->SetSelectedIndex(OverallLevel >= 0 ? OverallLevel : 4, false);
    }
}

// -------- Device --------
void UPGSettingMenuWidget::EnumerateAudioDevices()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Output Device (비동기)
    FOnAudioOutputDevicesObtained OutputDelegate;
    OutputDelegate.BindDynamic(this, &UPGSettingMenuWidget::OnOutputDevicesObtained);
    UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(World, OutputDelegate);

    // Input Device (동기)
    PopulateInputDevices();
}

void UPGSettingMenuWidget::OnOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
    if (!OutputDeviceComboBox)
    {
        return;
    }

    bIsLoadingSettings = true;

    OutputDeviceComboBox->ClearOptions();
    OutputDeviceNameToId.Empty();

    UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
    FString SavedDeviceId = Settings ? Settings->OutputDeviceId : FString();
    FString DefaultName;
    FString SelectedName;

    for (const FAudioOutputDeviceInfo& Device : AvailableDevices)
    {
        FString DisplayName = Device.Name;
        FString DeviceId = Device.DeviceId;

        OutputDeviceNameToId.Add(DisplayName, DeviceId);
        OutputDeviceComboBox->AddOption(DisplayName);

        if (Device.bIsSystemDefault)
        {
            DefaultName = DisplayName;
        }

        if (Device.DeviceId == SavedDeviceId)
        {
            SelectedName = DisplayName;
        }
    }

    OutputDeviceComboBox->SetSelectedOption(SelectedName.IsEmpty() ? DefaultName : SelectedName);

    // 저장된 디바이스가 없으면 현재 선택된 디바이스를 저장
    if (Settings && Settings->OutputDeviceId.IsEmpty())
    {
        FString SelectedOption = OutputDeviceComboBox->GetSelectedOption();
        if (FString* Id = OutputDeviceNameToId.Find(SelectedOption))
        {
            Settings->OutputDeviceId = *Id;
            ApplyAndSaveSettings();
        }
    }

    bIsLoadingSettings = false;
}

void UPGSettingMenuWidget::PopulateInputDevices()
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
    FString DefaultName;
    FString SelectedName;

    for (const Audio::FCaptureDeviceInfo& Device : Devices)
    {
        FString DisplayName = Device.DeviceName;
        FString DeviceId = Device.DeviceId;

        InputDeviceNameToId.Add(DisplayName, DeviceId);
        InputDeviceComboBox->AddOption(DisplayName);

        if (DefaultName.IsEmpty())
        {
            DefaultName = DisplayName;
        }

        if (DeviceId == SavedDeviceId)
        {
            SelectedName = DisplayName;
        }
    }

    InputDeviceComboBox->SetSelectedOption(SelectedName.IsEmpty() ? DefaultName : SelectedName);

    // 저장된 디바이스가 없으면 현재 선택된 디바이스를 저장
    if (Settings && Settings->InputDeviceId.IsEmpty())
    {
        FString SelectedOption = InputDeviceComboBox->GetSelectedOption();
        if (FString* Id = InputDeviceNameToId.Find(SelectedOption))
        {
            Settings->InputDeviceId = *Id;
            ApplyAndSaveSettings();
        }
    }

    bIsLoadingSettings = false;
}

void UPGSettingMenuWidget::OnOutputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsLoadingSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[OutputDeviceChanged] is loading"));
        return;
    }

    FString* DeviceId = OutputDeviceNameToId.Find(SelectedItem);
    if (!DeviceId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[OutputDeviceChanged] no valid id"));
        return;
    }

    FOnCompletedDeviceSwap SwapDelegate;
    SwapDelegate.BindDynamic(this, &UPGSettingMenuWidget::OnOutputDeviceSwapComplete);
    UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), *DeviceId, SwapDelegate);

    UE_LOG(LogTemp, Log, TEXT("[OutputDeviceChanged] swap device [%s]"), **DeviceId);

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->OutputDeviceId = *DeviceId;
        ApplyAndSaveSettings();
    }
}

void UPGSettingMenuWidget::OnOutputDeviceSwapComplete(const FSwapAudioOutputResult& SwapResult)
{
    UE_LOG(LogTemp, Log, TEXT("[OutputDevice] Swap result - %s, Current: %s, Requested: %s"),
        SwapResult.Result == ESwapAudioOutputDeviceResultState::Success ? TEXT("Success") : TEXT("Failed"),
        *SwapResult.CurrentDeviceId,
        *SwapResult.RequestedDeviceId);
}

void UPGSettingMenuWidget::OnInputDeviceSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (bIsLoadingSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InputDeviceChanged] is loading"));
        return;
    }

    FString* DeviceId = InputDeviceNameToId.Find(SelectedItem);
    if (!DeviceId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[InputDeviceChanged] no valid id"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[InputDeviceChanged] swap device [%s]"), **DeviceId);

    // 마이크 디바이스 전환
    PGVoiceUtils::ChangeInputDevice(GetWorld(), SelectedItem);

    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->InputDeviceId = *DeviceId;
        ApplyAndSaveSettings();
    }
}

// -------- WidgetSwitch --------
void UPGSettingMenuWidget::OnGamePlayOptionButtonClicked()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);
    }
}

void UPGSettingMenuWidget::OnAudioOptionButtonClicked()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(1);
    }
}

void UPGSettingMenuWidget::OnVideoOptionButtonClicked()
{
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(2);
    }
}

// -------- Helper --------
void UPGSettingMenuWidget::ApplyAndSaveSettings()
{
    if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
    {
        Settings->ApplySettings(true);
        Settings->SaveSettings();
    }
}

float UPGSettingMenuWidget::MicSensitivityToSlider(float Threshold) const
{
    // Threshold 낮을수록 민감 -> 슬라이더 높을수록 Threshold는 낮은값
    const float Clamped = FMath::Clamp(Threshold, MicSensitivityMin, MicSensitivityMax);
    return (1.0f - FMath::GetRangePct(MicSensitivityMin, MicSensitivityMax, Clamped));
}

float UPGSettingMenuWidget::SliderToMicSensitivity(float SliderValue) const
{
    // 슬라이더 1.0 = 가장 민감(threshold 낮음), 0.0 = 둔감(threshold 높음)
    return FMath::Lerp(MicSensitivityMax, MicSensitivityMin, SliderValue);
}
