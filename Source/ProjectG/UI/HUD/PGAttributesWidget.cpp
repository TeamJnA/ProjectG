// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAttributesWidget.h"
#include "Components/Image.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"
#include "PGLogChannels.h"


void UPGAttributesWidget::NativeDestruct()
{
	if (LastBoundASC.IsValid())
	{
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetMaxSanityAttribute()).Remove(MaxSanityChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetSanityAttribute()).Remove(SanityChangedHandle);

		if (SanityRecoverTagHandle.IsValid())
		{
			LastBoundASC->RegisterGameplayTagEvent(SanityRecoverStateTag, EGameplayTagEventType::NewOrRemoved).Remove(SanityRecoverTagHandle);
			SanityRecoverTagHandle.Reset();
		}
	}

	// 적용중인 이펙트들도 초기화.( 다이나믹 마테리얼 값을 원상복구 후 저장 )
	// MID 를 Original 상태로 변경
	if (BorderMID)
	{
		BorderMID->SetVectorParameterValue(GlowColorParamName, FLinearColor(0.03f, 0.05f, 0.07f));
		BorderMID->SetVectorParameterValue(MetalLightColorParamName, FLinearColor(0.055f, 0.06f, 0.065f));
		BorderMID->SetScalarParameterValue(PulseMinBrightnessParamName, 0.8f);
		BorderMID->SetScalarParameterValue(PulseSpeedParamName, 1.0f);
	}

	Super::NativeDestruct();
}

/*
* 플레이어 스태미나 바인드
*/
void UPGAttributesWidget::BindToAttributes()
{
	APGPlayerState* PS = Cast<APGPlayerState>(GetOwningPlayerState());
	if (!PS) 
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributeWidget] No valid PS"));
		return;
	}

	UE_LOG(LogPGWidget, Log, TEXT("UPGAttributesWidget::BindToAttributes [%s]"), *(PS->GetName()));

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	UPGAttributeSet* AS = PS->GetAttributeSet();
	if (!ASC || !AS)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributeWidget] No valid ASC or AS"));
		return;
	}

	// 이전에 Bound한 내역이 있으면 제거 후 다시 바운드 ( ex.사망 후 부활 시 )
	if (LastBoundASC.IsValid())
	{
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).Remove(SanityChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxSanityAttribute()).Remove(MaxSanityChangedHandle);

		if (SanityRecoverTagHandle.IsValid())
		{
			LastBoundASC->RegisterGameplayTagEvent(SanityRecoverStateTag, EGameplayTagEventType::NewOrRemoved).Remove(SanityRecoverTagHandle);
			SanityRecoverTagHandle.Reset();
		}
	}

	// Initial Attributes
	RefreshSanity(AS->GetSanityAttribute().GetNumericValue(AS));
	RefreshMaxSanity(AS->GetMaxSanityAttribute().GetNumericValue(AS));

	// Sanity Change
	SanityChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			RefreshSanity(Data.NewValue);
		});

	// MaxSanity Change
	MaxSanityChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxSanityAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			RefreshMaxSanity(Data.NewValue);
		});

	// Bind Sanity recover Tag change
	SanityRecoverTagHandle = ASC->RegisterGameplayTagEvent(
		SanityRecoverStateTag,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UPGAttributesWidget::OnSanityRecover);

	LastBoundASC = ASC;
}

void UPGAttributesWidget::RefreshSanity(float InSanity)
{
	/* New Logic Image 
	*  앨리어싱(계단 현상)을 방지하기 위해 Progress Bar 대신 Image로 현재 정신력 수치를 표현한다.
	* 5.5 이전 버전까지는 ProgressBar를 Shear 했을 때 깨짐 현상(계단)이 발생함에 대한 해결법이 없음.
	* Retainer Box가 정석 해결법이나, 5.5 이전 버전에는 Progress Bar와 충돌이 발생.
	*/
	if (!CurSanityBarMID)
	{
		if (SanityCurrentBar)
		{
			CurSanityBarMID = SanityCurrentBar->GetDynamicMaterial();
		}
	}

	float SanityCurPercent = FMath::Clamp(InSanity / SanityDisplayMax, 0.0f, 1.0f);

	if (CurSanityBarMID)
	{
		CurSanityBarMID->SetScalarParameterValue(PercentParam, SanityCurPercent);

		// 40 이하일 경우 1.0, 아니면 0.2
		const float NewScalarValue = (InSanity <= 40.0f) ? 1.0f : 0.2f;
		CurSanityBarMID->SetScalarParameterValue(NoiseFlipSpeedParam, NewScalarValue);

		// 60~40 구간에서 (1,1,1) -> (0.1,0.1,0.1) 선형 보간, 범위 밖은 클램프
		const float Alpha = FMath::Clamp((InSanity - 40.0f) / (60.0f - 40.0f), 0.0f, 1.0f);
		const FLinearColor NewColorValue = FMath::Lerp( FLinearColor(0.1f, 0.1f, 0.1f, 1.0f), FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), Alpha);
		CurSanityBarMID->SetVectorParameterValue(NoiseDarkParam, NewColorValue);
	}

	const bool bNewDark = (InSanity <= 40.0f);
	if (bNewDark != bIsSanityDark)
	{
		bIsSanityDark = bNewDark;
		SetOutLine();
	}
}

void UPGAttributesWidget::RefreshMaxSanity(float InMaxSanity)
{
	if (!SanityLockedMID)
	{
		if (SanityLockedBar)
		{
			SanityLockedMID = SanityLockedBar->GetDynamicMaterial();
		}
	}

	const float MaxRatio = FMath::Clamp(InMaxSanity / SanityDisplayMax, 0.0f, 1.0f);
	float LockedPercent = MaxRatio;

	if (SanityLockedMID)
	{
		SanityLockedMID->SetScalarParameterValue(PercentParam, LockedPercent);
	}
}

void UPGAttributesWidget::OnSanityRecover(const FGameplayTag Tag, int32 NewCount)
{
	UE_LOG(LogPGWidget, Log, TEXT("On sanity recover changed to %d"), NewCount);

	if (!CurSanityBarMID)
	{
		if (SanityCurrentBar)
		{
			CurSanityBarMID = SanityCurrentBar->GetDynamicMaterial();
		}
	}

	if (CurSanityBarMID)
	{
		CurSanityBarMID->SetScalarParameterValue(SanityRecoverParameterName, NewCount);
	}

	const bool bNewRecovering = (NewCount == 1);
	if (bNewRecovering != bIsSanityRecovering)
	{
		bIsSanityRecovering = bNewRecovering;
		SetOutLine();
	}
}

void UPGAttributesWidget::SetOutLine()
{
	if (!BorderMID)
	{
		if (Border)
		{
			BorderMID = Border->GetDynamicMaterial();
		}
	}

	if (!BorderMID)
	{
		return;
	}

	// 4가지 조합: Origin / Recover / Dark / Dark & Recover
	if (bIsSanityDark && bIsSanityRecovering)
	{
		// [Dark & Recover]
		BorderMID->SetVectorParameterValue(GlowColorParamName, FLinearColor(0.25f, 0.0f, 0.0f));
		BorderMID->SetVectorParameterValue(MetalLightColorParamName, FLinearColor(0.0f, 0.0f, 0.0f));
		BorderMID->SetScalarParameterValue(PulseMinBrightnessParamName, 0.0f);
		BorderMID->SetScalarParameterValue(PulseSpeedParamName, 2.0f);
	}
	else if (bIsSanityDark)
	{
		// [Dark]
		BorderMID->SetVectorParameterValue(GlowColorParamName, FLinearColor(0.0f, 0.0f, 0.0f));
		BorderMID->SetVectorParameterValue(MetalLightColorParamName, FLinearColor(0.0f, 0.0f, 0.0f));
	}
	else if (bIsSanityRecovering)
	{
		// [Recover]
		BorderMID->SetVectorParameterValue(GlowColorParamName, FLinearColor(0.25f, 0.0f, 0.0f));
		BorderMID->SetScalarParameterValue(PulseMinBrightnessParamName, 0.0f);
		BorderMID->SetScalarParameterValue(PulseSpeedParamName, 2.0f);
	}
	else
	{
		// [Origin]
		BorderMID->SetVectorParameterValue(GlowColorParamName, FLinearColor(0.03f, 0.05f, 0.07f));
		BorderMID->SetVectorParameterValue(MetalLightColorParamName, FLinearColor(0.055f, 0.06f, 0.065f));
		BorderMID->SetScalarParameterValue(PulseMinBrightnessParamName, 0.8f);
		BorderMID->SetScalarParameterValue(PulseSpeedParamName, 1.0f);
	}
}


/*
[Origin]
PulseMinBrightness 0.8

GlowColor
0.03
0.05
0.07

PulseSpeed
1


****** 
[Recover]
PulseMinBrightness 0

GlowColor
0.25
0.0
0.0

PulseSpeed 
2

********
[Dark]
GlowColor
0.0
0.0
0.0

MetalLightColor
0.0
0.0
0.0

******
[Dark & Recover]
GlowColor
0.25
0.0
0.0

MetalLightColor
0.0
0.0
0.0
*/