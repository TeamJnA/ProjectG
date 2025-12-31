// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Screen/PGJumpscareWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

void UPGJumpscareWidget::Init(UTexture2D* InTexture)
{
	if (JumpscareImage && InTexture)
	{
		JumpscareImage->SetBrushFromTexture(InTexture);
	}

	if (JumpscareAnim)
	{
		UE_LOG(LogTemp, Log, TEXT("[JumpscareWidget] Anim bind"));

		PlayAnimation(JumpscareAnim);
	}
	else
	{
		RemoveFromParent();
	}
}

void UPGJumpscareWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == JumpscareAnim)
	{
		UE_LOG(LogTemp, Log, TEXT("[JumpscareWidget] Anim done"));
		RemoveFromParent();
	}
}
