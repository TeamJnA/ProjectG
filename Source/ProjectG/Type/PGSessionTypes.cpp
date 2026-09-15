#include "Type/PGSessionTypes.h"
#include "Misc/Base64.h"


namespace
{
    const FString Base64Prefix = TEXT("b64:");
}

FString PGSessionName::Encode(const FString& InName)
{
    if (InName.IsEmpty())
    {
        return FString();
    }

    FTCHARToUTF8 Converted(*InName);
    return Base64Prefix + FBase64::Encode((const uint8*)Converted.Get(), Converted.Length());
}

FString PGSessionName::Decode(const FString& InRaw)
{
    if (!InRaw.StartsWith(Base64Prefix))
    {
        return InRaw; // 구버전 세션 호환
    }

    TArray<uint8> Bytes;
    if (!FBase64::Decode(InRaw.RightChop(Base64Prefix.Len()), Bytes))
    {
        return FString();
    }

    Bytes.Add(0); // 널 종단
    return FString(UTF8_TO_TCHAR((const ANSICHAR*)Bytes.GetData()));
}
