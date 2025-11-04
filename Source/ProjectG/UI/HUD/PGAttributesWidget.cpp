// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGAttributesWidget.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"

/*
* 플레이어 스태미나 바인드
*/
void UPGAttributesWidget::BindToAttributes()
{
	const APGPlayerState* PS = Cast<APGPlayerState>(GetOwningPlayerState());
	if (!PS) 
	{
		return;
	}

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	const UPGAttributeSet* AS = PS->GetAttributeSet();

	// Initial Attribute
	StaminaPercent = AS->GetStaminaAttribute().GetNumericValue(AS) / AS->GetMaxStaminaAttribute().GetNumericValue(AS);
	SanityPercent = AS->GetSanityAttribute().GetNumericValue(AS) / AS->GetMaxSanityAttribute().GetNumericValue(AS);

	// Attribute Changes
	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).AddLambda([this, AS](const FOnAttributeChangeData& Data)
	{
		StaminaPercent = Data.NewValue / AS->GetMaxStaminaAttribute().GetNumericValue(AS);
	});

	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetSanityAttribute()).AddLambda([this, AS](const FOnAttributeChangeData& Data)
	{
		SanityPercent = Data.NewValue / AS->GetMaxSanityAttribute().GetNumericValue(AS);
	});
}
