// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PGAttributesWidget.generated.h"

class UAbilitySystemComponent;
class UImage;

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

	void OnSanityRecover(const FGameplayTag Tag, int32 NewCount);
	
	FDelegateHandle SanityChangedHandle;
	FDelegateHandle MaxSanityChangedHandle;
	FDelegateHandle SanityRecoverTagHandle;

	TWeakObjectPtr<UAbilitySystemComponent> LastBoundASC;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SanityCurrentBar;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CurSanityBarMID;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SanityLockedBar;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SanityLockedMID;

	UPROPERTY(EditDefaultsOnly, Category = "Indicator")
	FName PercentParam = TEXT("Percent");

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	FLinearColor SanityNormalColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	FLinearColor SanityLowColor = FLinearColor(0.15f, 0.0f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float SanityDisplayMax = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	float SanityLowColorThreshold = 40.0f;

	/* Sanity Recover */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity")
	FGameplayTag SanityRecoverStateTag;

	// 파라미터 이름은 바뀌지 않으므로 기본값을 설정해두면 좋습니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity")
	FName SanityRecoverParameterName = FName("bPanner");
};
