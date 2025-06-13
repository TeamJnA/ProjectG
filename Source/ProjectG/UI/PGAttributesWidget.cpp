// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGAttributesWidget.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"

void UPGAttributesWidget::BindToAttributes()
{
	const APGPlayerState* PS = Cast<APGPlayerState>(GetOwningPlayerState());
	if (!PS) return;

	UE_LOG(LogTemp, Log, TEXT("%s PlayerState bind"), *PS->GetName());

	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	const UPGAttributeSet* AS = PS->GetAttributeSet();

	// Initial Attribute
	StaminaPercent = AS->GetStaminaAttribute().GetNumericValue(AS) / AS->GetMaxStaminaAttribute().GetNumericValue(AS);

	// Attribute Changes
	ASC->GetGameplayAttributeValueChangeDelegate(AS->GetStaminaAttribute()).AddLambda([this, AS](const FOnAttributeChangeData& Data)
	{
		StaminaPercent = Data.NewValue / AS->GetMaxStaminaAttribute().GetNumericValue(AS);
	});
}
