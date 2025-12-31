// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PGJumpscareWidget.generated.h"

class UImage;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGJumpscareWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Init(UTexture2D* InTexture);

protected:
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> JumpscareImage;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> JumpscareAnim;
};
