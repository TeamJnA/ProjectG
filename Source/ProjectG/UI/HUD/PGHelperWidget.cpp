// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGHelperWidget.h"
#include "UI/HUD/PGHelperExitEntryWidget.h"
#include "Type/PGHelperTypes.h"
#include "Game/PGGameState.h"
#include "Player/PGPlayerState.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Components/HorizontalBox.h"
#include "Engine/DataTable.h"


void UPGHelperWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	FWidgetAnimationDynamicEvent Finished;
	Finished.BindDynamic(this, &UPGHelperWidget::HandleSlideAnimFinished);

	if (SlideInAnim)
	{
		BindToAnimationFinished(SlideInAnim, Finished);
	}

	if (SlideOutAnim)
	{
		BindToAnimationFinished(SlideOutAnim, Finished);
	}
}

void UPGHelperWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!bIsOpen)
	{
		SnapToClosedState();
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UPGHelperWidget::NativeDestruct()
{
	bIsOpen = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
		World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
	}

	Super::NativeDestruct();
}

void UPGHelperWidget::OpenWithAutoClose()
{
	if (bIsOpen)
	{
		return;
	}
	bIsOpen = true;

	Refresh();

	if (SlideOutAnim && IsAnimationPlaying(SlideOutAnim))
	{
		StopAnimation(SlideOutAnim);
	}

	if (SlideInAnim)
	{
		PlayAnimation(SlideInAnim, 0.0f, 1);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AutoCloseTimerHandle, this, &UPGHelperWidget::CloseIfOpen, AutoCloseDelay, false);
	}
}

void UPGHelperWidget::CloseIfOpen()
{
	if (!bIsOpen)
	{
		return;
	}
	bIsOpen = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
		World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
	}

	if (SlideInAnim && IsAnimationPlaying(SlideInAnim))
	{
		StopAnimation(SlideInAnim);
	}

	if (SlideOutAnim)
	{
		PlayAnimation(SlideOutAnim, 0.0f, 1);
	}
}

void UPGHelperWidget::CloseAndCollapse()
{
	if (bIsOpen)
	{
		bIsOpen = false;

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
			World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
		}
	}

	SetVisibility(ESlateVisibility::Collapsed);
}

void UPGHelperWidget::RestoreFromCameraMode()
{
	StopAnimation(SlideInAnim);
	StopAnimation(SlideOutAnim);

	SnapToClosedState();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPGHelperWidget::ForceClose()
{
	bIsOpen = false;
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
		World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
	}

	StopAnimation(SlideInAnim);
	StopAnimation(SlideOutAnim);

	if (ExitListBox)
	{
		ExitListBox->ClearChildren();
	}
	ActiveEntries.Reset();
	PendingRows.Reset();
	NextPendingIndex = 0;

	SnapToClosedState();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UPGHelperWidget::HandleSlideAnimFinished()
{
	UE_LOG(LogTemp, Log, TEXT("Helper::HandleSlideAnimFinished: %d"), bIsOpen);
	if (!bIsOpen)
	{
		SnapToClosedState();
	}
}

void UPGHelperWidget::SnapToClosedState()
{
	if (HelperListRoot)
	{
		HelperListRoot->SetRenderTranslation(ListClosedTranslation);
	}

	if (HelperPeek)
	{
		HelperPeek->SetRenderOpacity(1.0f);
	}
}

void UPGHelperWidget::NotifyCapturedChanged()
{
	if (bIsOpen)
	{
		Refresh();
	}
}

void UPGHelperWidget::NotifyExitLockChanged()
{
	if (!bIsOpen)
	{
		return;
	}

	if (IsIntroPlaying())
	{
		Refresh();
	}
	else
	{
		UpdateInPlace();
	}
}

bool UPGHelperWidget::IsIntroPlaying() const
{
	if (NextPendingIndex < PendingRows.Num())
	{
		return true;
	}

	for (const TWeakObjectPtr<UPGHelperExitEntryWidget>& Weak : ActiveEntries)
	{
		if (UPGHelperExitEntryWidget* Entry = Weak.Get())
		{
			if (!Entry->IsIntroComplete())
			{
				return true;
			}
		}
	}

	return false;
}

void UPGHelperWidget::Refresh()
{
	if (!CatalogTable || !ExitListBox)
	{
		return;
	}

	ResolvePendingRows();

	ExitListBox->ClearChildren();
	ActiveEntries.Reset();
	NextPendingIndex = 0;

	if (PendingRows.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RowAppearTimerHandle, this, &UPGHelperWidget::AppearNextRow, RowAppearInterval, true);
	}
}

void UPGHelperWidget::ResolvePendingRows()
{
	PendingRows.Reset();

	APlayerController* PC = GetOwningPlayer();
	APGPlayerState* PS = PC ? PC->GetPlayerState<APGPlayerState>() : nullptr;
	if (!PS)
	{
		return;
	}

	TSet<int32> CapturedKeys;
	for (int32 ID : PS->GetCapturedIDs())
	{
		CapturedKeys.Add(PhotoID::GetSpeciesKey(ID));
	}

	TMap<int32, APGExitPointBase*> ExitByKey;
	BuildExitByKeyMap(ExitByKey);

	for (const auto& Pair : CatalogTable->GetRowMap())
	{
		const FPGHelperEntryRow* Row = reinterpret_cast<const FPGHelperEntryRow*>(Pair.Value);
		if (!Row)
		{
			continue;
		}

		const int32 SpeciesKey = FCString::Atoi(*Pair.Key.ToString());
		if (SpeciesKey == 0)
		{
			continue;
		}

		if (!Row->bDefaultVisible && !CapturedKeys.Contains(SpeciesKey))
		{
			continue;
		}

		FPGHelperPendingRow PendingRow;
		PendingRow.SpeciesKey = SpeciesKey;
		PendingRow.Row = *Row;

		if (APGExitPointBase* Exit = ExitByKey.FindRef(SpeciesKey))
		{
			PendingRow.UnlockedCounts = Exit->GetUnlockedItemCounts();
			PendingRow.bDepleted = Exit->IsExitDepleted();
		}

		PendingRows.Add(MoveTemp(PendingRow));
	}

	PendingRows.Sort([](const FPGHelperPendingRow& A, const FPGHelperPendingRow& B)
		{
			return A.Row.DisplayOrder < B.Row.DisplayOrder;
		});
}

void UPGHelperWidget::AppearNextRow()
{
	if (NextPendingIndex >= PendingRows.Num())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
		}
		return;
	}

	if (!ExitEntryWidgetClass || !ExitListBox)
	{
		return;
	}

	const FPGHelperPendingRow& PendingRow = PendingRows[NextPendingIndex++];

	UPGHelperExitEntryWidget* Entry = CreateWidget<UPGHelperExitEntryWidget>(this, ExitEntryWidgetClass);
	if (!Entry)
	{
		return;
	}

	Entry->SetEntry(PendingRow.SpeciesKey, PendingRow.Row, PendingRow.UnlockedCounts, PendingRow.bDepleted);
	ExitListBox->AddChildToHorizontalBox(Entry);
	Entry->PlayIntroAnim();

	ActiveEntries.Add(Entry);
}

void UPGHelperWidget::UpdateInPlace()
{
	TMap<int32, APGExitPointBase*> ExitByKey;
	BuildExitByKeyMap(ExitByKey);

	for (const TWeakObjectPtr<UPGHelperExitEntryWidget>& WeakEntry : ActiveEntries)
	{
		UPGHelperExitEntryWidget* Entry = WeakEntry.Get();
		if (!Entry)
		{
			continue;
		}

		TMap<EPGExitItemType, int32> UnlockedCounts;
		bool bDepleted = false;
		if (APGExitPointBase* Exit = ExitByKey.FindRef(Entry->GetSpeciesKey()))
		{
			UnlockedCounts = Exit->GetUnlockedItemCounts();
			bDepleted = Exit->IsExitDepleted();
		}

		Entry->UpdateInPlace(UnlockedCounts, bDepleted);
	}
}

void UPGHelperWidget::BuildExitByKeyMap(TMap<int32, APGExitPointBase*>& OutExitByKey) const
{
	OutExitByKey.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APGGameState* GS = World->GetGameState<APGGameState>();
	if (!GS)
	{
		return;
	}

	for (const auto& Pair : GS->GetExitsBySpeciesKey())
	{
		if (APGExitPointBase* Exit = Pair.Value.Get())
		{
			OutExitByKey.Add(Pair.Key, Exit);
		}
	}
}
