#include "Type/PGLanguageTypes.h"

namespace
{
    struct FLanguageNames
    {
        const TCHAR* Full;
        const TCHAR* Short;
    };

    const TMap<FString, FLanguageNames> LanguageDisplayNames = {
        { TEXT("de"),      { TEXT("DEUTSCH"),           TEXT("DEUTSCH")    } },
        { TEXT("en"),      { TEXT("ENGLISH"),           TEXT("ENGLISH")    } },
        { TEXT("es-es"),   { TEXT("ESPAÑOL (ESPAÑA)"),  TEXT("ESPAÑOL")    } },
        { TEXT("es-419"),  { TEXT("ESPAÑOL (LATAM)"),   TEXT("ESPAÑOL")    } },
        { TEXT("fr"),      { TEXT("FRANÇAIS"),          TEXT("FRANÇAIS")   } },
        { TEXT("id"),      { TEXT("INDONESIA"),         TEXT("INDONESIA")  } },
        { TEXT("it"),      { TEXT("ITALIANO"),          TEXT("ITALIANO")   } },
        { TEXT("pl"),      { TEXT("POLSKI"),            TEXT("POLSKI")     } },
        { TEXT("pt-br"),   { TEXT("PORTUGUÊS (BR)"),    TEXT("PORTUGUÊS")  } },
        { TEXT("tr"),      { TEXT("TÜRKÇE"),            TEXT("TÜRKÇE")     } },
        { TEXT("vi"),      { TEXT("TIẾNG VIỆT"),        TEXT("TIẾNG VIỆT") } },
        { TEXT("ru"),      { TEXT("РУССКИЙ"),           TEXT("РУССКИЙ")    } },
        { TEXT("th"),      { TEXT("ไทย"),               TEXT("ไทย")        } },
        { TEXT("ja"),      { TEXT("日本語"),            TEXT("日本語")     } },
        { TEXT("ko"),      { TEXT("한국어"),            TEXT("한국어")     } },
        { TEXT("zh-hans"), { TEXT("中文（简体）"),      TEXT("中文（简体）") } },
        { TEXT("zh-hant"), { TEXT("中文（繁體）"),      TEXT("中文（繁體）") } },
    };

    FText LookupName(const FString& InCulture, bool bShort)
    {
        const FString Code = PGLanguage::NormalizeCode(InCulture);
        if (const FLanguageNames* Found = LanguageDisplayNames.Find(Code.ToLower()))
        {
            return FText::FromString(bShort ? Found->Short : Found->Full);
        }
        return FText::FromString(Code.ToUpper());
    }
}

FString PGLanguage::NormalizeCode(const FString& InCulture)
{
    FString Culture = InCulture.Replace(TEXT("_"), TEXT("-"));

    TArray<FString> Parts;
    Culture.ParseIntoArray(Parts, TEXT("-"), true);
    if (Parts.Num() == 0)
    {
        return TEXT("en");
    }

    const FString Primary = Parts[0].ToLower();

    if ((Primary == TEXT("zh") || Primary == TEXT("pt") || Primary == TEXT("es")) && Parts.Num() >= 2)
    {
        return Primary + TEXT("-") + Parts[1];
    }

    return Primary;
}

FText PGLanguage::CodeToDisplayText(const FString& InCulture)
{
    return LookupName(InCulture, false);
}

FText PGLanguage::CodeToShortText(const FString& InCulture)
{
    return LookupName(InCulture, true);
}
