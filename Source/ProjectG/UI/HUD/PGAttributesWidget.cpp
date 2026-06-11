// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAttributesWidget.h"
#include "Components/ProgressBar.h"
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
	SanityValue = InSanity;
	SanityPercent = FMath::Clamp(InSanity / SanityDisplayMax, 0.0f, 1.0f);

	if (SanityBar)
	{
		const FLinearColor BarColor = (InSanity <= SanityLowColorThreshold) ? SanityLowColor : SanityNormalColor;
		SanityBar->SetFillColorAndOpacity(BarColor);
	}
}

void UPGAttributesWidget::RefreshMaxSanity(float InMaxSanity)
{
	MaxSanityValue = InMaxSanity;
	const float MaxRatio = FMath::Clamp(InMaxSanity / SanityDisplayMax, 0.0f, 1.0f);
	LockedPercent = 1.0f - MaxRatio;
}
