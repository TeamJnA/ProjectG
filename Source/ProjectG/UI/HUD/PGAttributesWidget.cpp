// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAttributesWidget.h"
#include "Components/Image.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"


void UPGAttributesWidget::NativeDestruct()
{
	if (LastBoundASC.IsValid())
	{
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetMaxSanityAttribute()).Remove(MaxSanityChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetSanityAttribute()).Remove(SanityChangedHandle);
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

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	UPGAttributeSet* AS = PS->GetAttributeSet();
	if (!ASC || !AS)
	{
		UE_LOG(LogTemp, Error, TEXT("[AttributeWidget] No valid ASC or AS"));
		return;
	}

	if (LastBoundASC.IsValid())
	{
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).Remove(SanityChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetMaxSanityAttribute()).Remove(MaxSanityChangedHandle);
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
