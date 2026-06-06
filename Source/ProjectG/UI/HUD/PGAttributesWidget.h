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

	void RefreshSanity(float InSanity);
	void RefreshMaxSanity(float InMaxSanity);

	FDelegateHandle SanityChangedHandle;
	FDelegateHandle MaxSanityChangedHandle;

	TWeakObjectPtr<UAbilitySystemComponent> LastBoundASC;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float SanityDisplayMax = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	float SanityPercent;

	UPROPERTY(BlueprintReadOnly)
	float LockedPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxSanityValue = 100.0f;

	UPROPERTY(BlueprintReadOnly)
	float SanityValue = 100.0f;
};
