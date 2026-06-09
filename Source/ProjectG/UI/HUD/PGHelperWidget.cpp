// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PGHelperWidget.h"
#include "UI/HUD/PGHelperExitEntryWidget.h"
#include "Type/PGHelperTypes.h"
#include "Player/PGPlayerState.h"
#include "Level/Exit/PGExitPointBase.h"
#include "Components/VerticalBox.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"


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

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	APGPlayerState* PS = PC->GetPlayerState<APGPlayerState>();
	if (!PS)
	{
		return;
	}

	TMap<int32, APGExitPointBase*> ExitByKey;
	BuildExitByKeyMap(ExitByKey);

	TSet<int32> CapturedKeys;
	for (int32 ID : PS->GetCapturedIDs())
	{
		CapturedKeys.Add(PhotoID::GetSpeciesKey(ID));
	}

	struct FResolvedRow
	{
		int32 SpeciesKey;
		const FPGHelperEntryRow* Row;
	};
	TArray<FResolvedRow> ResolvedRows;

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

		const bool bUnlocked = Row->bDefaultVisible || CapturedKeys.Contains(SpeciesKey);
		if (!bUnlocked)
		{
			continue;
		}

		ResolvedRows.Add({ SpeciesKey, Row });
	}

	ResolvedRows.Sort([](const FResolvedRow& A, const FResolvedRow& B)
	{
		return A.Row->DisplayOrder < B.Row->DisplayOrder;
	});

	PendingRows.Reset();
	for (const FResolvedRow& ResolvedRow : ResolvedRows)
	{
		FPendingRow PendingRow;
		PendingRow.SpeciesKey = ResolvedRow.SpeciesKey;
		PendingRow.Row = *ResolvedRow.Row;
		if (APGExitPointBase* Exit = ExitByKey.FindRef(ResolvedRow.SpeciesKey))
		{
			PendingRow.UnlockedIds = Exit->GetUnlockedItemIds();
			PendingRow.bDepleted = Exit->IsExitDepleted();
		}
		PendingRows.Add(MoveTemp(PendingRow));
	}

	ExitListBox->ClearChildren();
	ActiveEntries.Reset();
	NextPendingIndex = 0;

	if (PendingRows.IsEmpty())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RowAppearTimerHandle);
	}

	if (NextPendingIndex < PendingRows.Num())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(RowAppearTimerHandle, this, &UPGHelperWidget::AppearNextRow, RowAppearInterval, true);
		}
	}
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

	const FPendingRow& PendingRow = PendingRows[NextPendingIndex++];

	UPGHelperExitEntryWidget* Entry = CreateWidget<UPGHelperExitEntryWidget>(this, ExitEntryWidgetClass);
	if (!Entry)
	{
		return;
	}

	Entry->SetEntry(PendingRow.SpeciesKey, PendingRow.Row, PendingRow.UnlockedIds, PendingRow.bDepleted);
	ExitListBox->AddChildToVerticalBox(Entry);
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

		TSet<FName> UnlockedIds;
		bool bDepleted = false;
		if (APGExitPointBase* Exit = ExitByKey.FindRef(Entry->GetSpeciesKey()))
		{
			UnlockedIds = Exit->GetUnlockedItemIds();
			bDepleted = Exit->IsExitDepleted();
		}

		Entry->UpdateInPlace(UnlockedIds, bDepleted);
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

	for (TActorIterator<APGExitPointBase> It(World); It; ++It)
	{
		APGExitPointBase* Exit = *It;
		if (!Exit)
		{
			continue;
		}

		const int32 SpeciesKey = Exit->GetLinkedSpeciesKey();
		if (SpeciesKey != 0)
		{
			OutExitByKey.Add(SpeciesKey, Exit);
		}
	}
}
