#include "PGVoiceUtils.h"

#include "Player/PGGameUserSettings.h"
#include "AudioCapture.h"
#include "Net/VoiceConfig.h"
#include "VoiceInterfaceImpl.h"
#include "VoiceEngineImpl.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/VoiceCapture.h"
#include "VoiceCaptureWindows.h"

struct FOnlineVoiceImplEx : public FOnlineVoiceImpl
{
    IVoiceEnginePtr GetVoiceEngine() const
    {
        return VoiceEngine;
    }
};

struct FVoiceEngineImplEx : public FVoiceEngineImpl
{
    TSharedPtr<IVoiceCapture> GetVoiceCapture_Public() const
    {
        return GetVoiceCapture();
    }
};

static FVoiceCaptureWindows* GetWinVoiceCapture(UWorld* World)
{
    IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(World);
    if (VoiceInterface)
    {
        if (IVoiceEnginePtr VoiceEngine = ((FOnlineVoiceImplEx*)VoiceInterface.Get())->GetVoiceEngine())
        {
            if (auto VoiceCapture = ((FVoiceEngineImplEx*)VoiceEngine.Get())->GetVoiceCapture_Public())
            {
                return (FVoiceCaptureWindows*)VoiceCapture.Get();
            }
        }
    }
    return nullptr;
}

void PGVoiceUtils::ApplySavedInputDevice(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PGVoiceUtils] No valid world"));
        return;
    }

    Audio::FAudioCapture AudioCapture;
    TArray<Audio::FCaptureDeviceInfo> Devices;
    AudioCapture.GetCaptureDevicesAvailable(Devices);

    if (Devices.IsEmpty())
    {        
        UE_LOG(LogTemp, Error, TEXT("[PGVoiceUtils] No valid device"));
        return;
    }

    UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("[PGVoiceUtils] No valid user setting"));
        return;
    }

    // 저장된 디바이스 찾기, 없으면 첫 번째 디바이스 지정
    const Audio::FCaptureDeviceInfo* TargetDevice = nullptr;
    if (!Settings->InputDeviceId.IsEmpty())
    {
        for (const Audio::FCaptureDeviceInfo& Device : Devices)
        {
            if (Device.DeviceId == Settings->InputDeviceId)
            {
                TargetDevice = &Device;
                break;
            }
        }
    }

    if (!TargetDevice)
    {
        TargetDevice = &Devices[0];
    }

    ChangeInputDevice(World, TargetDevice->DeviceName);
}

bool PGVoiceUtils::ChangeInputDevice(UWorld* World, const FString& DeviceName)
{
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PGVoiceUtils] No valid world"));
        return false;
    }

    FVoiceCaptureWindows* VoiceCapture = GetWinVoiceCapture(World);
    if (!VoiceCapture)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PGVoiceUtils] No valid voice capture"));
        return false;
    }

    IOnlineVoicePtr VoiceInterface = Online::GetVoiceInterface(World);
    if (VoiceInterface.IsValid())
    {
        VoiceInterface->StopNetworkedVoice(0);
    }

    VoiceCapture->ChangeDevice(DeviceName, UVOIPStatics::GetVoiceSampleRate(), UVOIPStatics::GetVoiceNumChannels());

    if (VoiceInterface.IsValid())
    {
        VoiceInterface->StartNetworkedVoice(0);
    }

    UE_LOG(LogTemp, Log, TEXT("[InputDevice] Changed to: %s"), *DeviceName);
    return true;
}
