#pragma once

#include "CoreMinimal.h"

class UWorld;

namespace PGVoiceUtils
{
    /** 시작 시 저장된 디바이스 적용 */
    void ApplySavedInputDevice(UWorld* World);

    /** 특정 디바이스로 즉시 전환 (DeviceName 기준) */
    bool ChangeInputDevice(UWorld* World, const FString& DeviceName);
}
