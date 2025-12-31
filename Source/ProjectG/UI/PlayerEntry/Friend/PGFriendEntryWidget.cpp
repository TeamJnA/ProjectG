// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/Friend/PGFriendEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Game/PGAdvancedFriendsGameInstance.h"

void UPGFriendEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (InviteButton)
	{
		InviteButton->OnClicked.AddDynamic(this, &UPGFriendEntryWidget::OnInviteButtonClicked);
	}
}

void UPGFriendEntryWidget::SetupFriendEntry(const FText& InFriendName, UTexture2D* InAvatarTexture, bool bIsOnline, TSharedPtr<const FUniqueNetId> InFriendNetId)
{
	if (FriendNameText)
	{
		FriendNameText->SetText(InFriendName);
	}

	if (FriendAvatar && InAvatarTexture)
	{
		FriendAvatar->SetBrushFromTexture(InAvatarTexture);
		FriendAvatar->SetVisibility(ESlateVisibility::Visible);
	}

	FriendNetId = InFriendNetId;

	const float Opacity = bIsOnline ? 1.0f : 0.5f;
	SetRenderOpacity(Opacity);

	if (InviteButton)
	{
		InviteButton->SetIsEnabled(bIsOnline);
	}
}

void UPGFriendEntryWidget::OnInviteButtonClicked()
{
	UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (GI && FriendNetId.IsValid())
	{
		GI->InviteFriend(*FriendNetId);
	}
}
