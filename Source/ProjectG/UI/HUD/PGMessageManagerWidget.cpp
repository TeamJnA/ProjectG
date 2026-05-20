// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGMessageManagerWidget.h"

#include "UI/Manager/PGHUD.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "Character/PGPlayerCharacter.h"
#include "Interface/InteractableActorInterface.h"


void UPGMessageManagerWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FailureIconTimerHandle);
	}

	if (APGPlayerCharacter* Character = CharacterRef.Get())
	{
		Character->OnStareTargetUpdate.RemoveAll(this);
	}

	Super::NativeDestruct();
}

/*
* 플레이어 메시지 위젯 바인드
* 바라보는 대상에 따른 메시지 디스플레이
*/
void UPGMessageManagerWidget::BindMessageEntry(APGPlayerCharacter* PlayerCharacter)
{
	if (!PlayerCharacter)
	{
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("UPGMessageManagerWidget::BindMessageEntry: InPlayerCharacter is valid. Binding delegate."));

	CharacterRef = PlayerCharacter;
	PlayerCharacter->OnStareTargetUpdate.RemoveAll(this);
	PlayerCharacter->OnStareTargetUpdate.AddDynamic(this, &UPGMessageManagerWidget::HandleOnStareTargetUpdate);
}

void UPGMessageManagerWidget::HandleOnStareTargetUpdate(AActor* TargetActor)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FailureIconTimerHandle);
	}

	SetFailureIcon(nullptr, FVector2D::ZeroVector);
	if (FailureXImage)
	{
		FailureXImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisible(true);
		}
	}

	if (InteractIconImage)
	{
		InteractIconImage->SetColorAndOpacity(NormalTint);
	}

	if (InteractText)
	{
		InteractText->SetColorAndOpacity(NormalTint);
	}

	if (TargetActor)
	{
		if (IInteractableActorInterface* Interactable = Cast<IInteractableActorInterface>(TargetActor))
		{
			SetInteractIcon(true);
			SetInteractText(Interactable->GetInteractionText());
		}
		else
		{
			SetInteractIcon(false);
			SetInteractText(FText::GetEmpty());
		}
	}
	else
	{
		SetInteractIcon(false);
		SetInteractText(FText::GetEmpty());
	}
}

void UPGMessageManagerWidget::SetInteractIcon(bool bShow)
{
	if (!InteractIconImage) 
	{
		return;
	}
	
	if (bShow && InteractIconTexture)
	{
		InteractIconImage->SetBrushFromTexture(InteractIconTexture);
		InteractIconImage->SetBrushSize(InteractIconSize);
		InteractIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		InteractIconImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPGMessageManagerWidget::SetInteractText(const FText& InText)
{
	if (!InteractText)
	{
		return;
	}

	if (!InText.IsEmpty())
	{
		InteractText->SetText(InText);
		InteractText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		InteractText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPGMessageManagerWidget::ShowFailureIcon(UMaterialInterface* Icon, FVector2D IconSize, float Duration, bool bAffectInteractPrompt)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FailureIconTimerHandle);
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisible(false);
		}
	}

	SetFailureIcon(Icon, IconSize);
	if (FailureXImage)
	{
		FailureXImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (bAffectInteractPrompt)
	{
		if (InteractIconImage)
		{
			InteractIconImage->SetColorAndOpacity(FailureTint);
		}

		if (InteractText)
		{
			InteractText->SetColorAndOpacity(FailureTint);
		}

		SetInteractText(FailedText);
	}
	else
	{
		if (InteractIconImage)
		{
			InteractIconImage->SetColorAndOpacity(NormalTint);
		}

		if (InteractText)
		{
			InteractText->SetColorAndOpacity(NormalTint);
		}

		RestoreInteractText();
	}

	if (UWorld* World = GetWorld())
	{
		if (Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(FailureIconTimerHandle, this, &UPGMessageManagerWidget::OnFailureTimerExpired, Duration, false);
		}
	}
}

void UPGMessageManagerWidget::SetFailureIcon(UMaterialInterface* Icon, FVector2D IconSize)
{
	if (!FailureIconImage)
	{
		return;
	}

	if (Icon)
	{
		FailureIconImage->SetBrushFromMaterial(Icon);
		FailureIconImage->SetBrushSize(IconSize);
		FailureIconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		FailureIconImage->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPGMessageManagerWidget::OnFailureTimerExpired()
{
	SetFailureIcon(nullptr, FVector2D::ZeroVector);
	if (FailureXImage)
	{
		FailureXImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (InteractIconImage)
	{
		InteractIconImage->SetColorAndOpacity(NormalTint);
	}

	if (InteractText)
	{
		InteractText->SetColorAndOpacity(NormalTint);
	}

	RestoreInteractText();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
		{
			HUD->SetCrosshairVisible(true);
		}
	}
}

void UPGMessageManagerWidget::RestoreInteractText()
{
	APGPlayerCharacter* Character = CharacterRef.Get();
	if (!Character)
	{
		SetInteractText(FText::GetEmpty());
		return;
	}

	AActor* CurrentTarget = Character->GetStaringTarget();
	if (CurrentTarget)
	{
		if (IInteractableActorInterface* Interactable = Cast<IInteractableActorInterface>(CurrentTarget))
		{
			SetInteractText(Interactable->GetInteractionText());
			return;
		}
	}

	SetInteractText(FText::GetEmpty());
}
