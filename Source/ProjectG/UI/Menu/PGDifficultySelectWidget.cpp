// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/PGDifficultySelectWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"


void UPGDifficultySelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (NormalButton)
	{
		NormalButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnNormalClicked);
	}

	if (HardButton)
	{
		HardButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnHardClicked);
	}

	if (Btn_Public)
	{
		Btn_Public->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnPublicClicked);
	}

	if (Btn_InviteOnly)
	{
		Btn_InviteOnly->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnInviteOnlyClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnCancelClicked);
	}

	if (HostGameButton)
	{
		HostGameButton->OnClicked.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnHostGameClicked);
	}

	if (ET_SessionName)
	{
		ET_SessionName->OnTextChanged.AddUniqueDynamic(this, &UPGDifficultySelectWidget::OnSessionNameChanged);
	}

	bIsDifNormal = true;
	bIsInviteOnly = false;
}

void UPGDifficultySelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	SetKeyboardFocus();

	if (ET_SessionName)
	{
		ET_SessionName->SetText(FText::GetEmpty());
		ET_SessionName->SetHintText(FText::FromString(GetDefaultSessionName()));
	}
}

FReply UPGDifficultySelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnCancelClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPGDifficultySelectWidget::OnNormalClicked()
{
	bIsDifNormal = true;
	// RemoveFromParent();
}

void UPGDifficultySelectWidget::OnHardClicked()
{
	bIsDifNormal = false;
	// RemoveFromParent();
}

void UPGDifficultySelectWidget::OnPublicClicked()
{
	bIsInviteOnly = false;
}

void UPGDifficultySelectWidget::OnInviteOnlyClicked()
{
	bIsInviteOnly = true;
}

void UPGDifficultySelectWidget::OnSessionNameChanged(const FText& NewText)
{
	if (!ET_SessionName)
	{
		return;
	}

	const FString Str = NewText.ToString();
	if (Str.Len() > PG_MAX_SESSION_NAME_LENGTH)
	{
		ET_SessionName->SetText(FText::FromString(Str.Left(PG_MAX_SESSION_NAME_LENGTH)));
	}
}

void UPGDifficultySelectWidget::OnCancelClicked()
{
	if (UUserWidget* Parent = ReturnFocusWidget.Get())
	{
		Parent->SetKeyboardFocus();
	}
	RemoveFromParent();
}

void UPGDifficultySelectWidget::OnHostGameClicked()
{
	FPGHostSessionOptions Options;
	Options.Difficulty = bIsDifNormal ? EPGDifficulty::Normal : EPGDifficulty::Hard;
	Options.bIsSinglePlay = bSinglePlayMode;
	Options.bIsInviteOnly = bSinglePlayMode ? true : bIsInviteOnly;
	Options.DisplayName = BuildSessionDisplayName();

	OnHostSessionConfirmed.Broadcast(Options);

	RemoveFromParent();
}

FString UPGDifficultySelectWidget::GetDefaultSessionName() const
{
	FString Nickname;
	if (const APlayerController* PC = GetOwningPlayer())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			Nickname = LP->GetNickname();
		}
	}
	Nickname.TrimStartAndEndInline();

	if (Nickname.IsEmpty())
	{
		return TEXT("Unknown Session");
	}

	const FString Suffix = TEXT("'s Session");
	const int32 MaxNicknameLen = PG_MAX_SESSION_NAME_LENGTH - Suffix.Len();
	if (MaxNicknameLen <= 0)
	{
		return Nickname.Left(PG_MAX_SESSION_NAME_LENGTH);
	}

	return Nickname.Left(MaxNicknameLen) + Suffix;
}

FString UPGDifficultySelectWidget::BuildSessionDisplayName() const
{
	FString Name = ET_SessionName ? ET_SessionName->GetText().ToString() : FString();
	Name.TrimStartAndEndInline();

	if (Name.IsEmpty())
	{
		return GetDefaultSessionName();
	}

	return Name.Left(PG_MAX_SESSION_NAME_LENGTH);
}
