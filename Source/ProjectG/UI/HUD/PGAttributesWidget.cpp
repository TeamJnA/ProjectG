// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAttributesWidget.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"

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
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).Remove(StaminaChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).Remove(SanityChangedHandle);
	}

	// Initial Attribute
	StaminaPercent = AS->GetStaminaAttribute().GetNumericValue(AS) / AS->GetMaxStaminaAttribute().GetNumericValue(AS);
	SanityPercent = AS->GetSanityAttribute().GetNumericValue(AS) / AS->GetMaxSanityAttribute().GetNumericValue(AS);

	// Attribute Changes
	StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).AddLambda([this, AS](const FOnAttributeChangeData& Data)
	{
		StaminaPercent = Data.NewValue / AS->GetMaxStaminaAttribute().GetNumericValue(AS);
	});

	SanityChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).AddLambda([this, AS](const FOnAttributeChangeData& Data)
	{
		SanityPercent = Data.NewValue / AS->GetMaxSanityAttribute().GetNumericValue(AS);
	});

	LastBoundASC = ASC;
}

void UPGAttributesWidget::NativeDestruct()
{
	if (LastBoundASC.IsValid())
	{
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
		LastBoundASC->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetSanityAttribute()).Remove(SanityChangedHandle);
	}

	Super::NativeDestruct();
}
