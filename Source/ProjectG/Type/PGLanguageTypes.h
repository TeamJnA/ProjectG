#pragma once

#include "CoreMinimal.h"

namespace PGLanguage
{
    /** "ko-KR" -> "ko". zh/pt/es 는 지역,표기 구분이 의미 있어 두 번째 토큰까지 유지 */
    FString NormalizeCode(const FString& InCulture);

    /** 세팅 메뉴용 - 지역까지 구분 */
    FText CodeToDisplayText(const FString& InCulture);

    /** 세션 슬롯용 - 짧은 표기 */
    FText CodeToShortText(const FString& InCulture);
}
