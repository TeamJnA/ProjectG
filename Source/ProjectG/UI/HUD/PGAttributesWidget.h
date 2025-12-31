// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGAttributesWidget.generated.h"

class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAttributesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindToAttributes();

protected:
	virtual void NativeDestruct() override;
	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle SanityChangedHandle;

	TWeakObjectPtr<UAbilitySystemComponent> LastBoundASC;

	UPROPERTY(BlueprintReadOnly)
	float StaminaPercent;

	UPROPERTY(BlueprintReadOnly)
	float SanityPercent;
};
