// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/Friend/PGFriendListWidget.h"
#include "UI/PlayerEntry/Friend/PGFriendEntryWidget.h"
#include "Components/VerticalBox.h"

#include "Game/PGAdvancedFriendsGameInstance.h"


void UPGFriendListWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GIRef = GI;
		GI->OnFriendListUpdated.AddDynamic(this, &UPGFriendListWidget::OnFriendListUpdated);
	}
}

void UPGFriendListWidget::NativeDestruct()
{
	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		GI->OnFriendListUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPGFriendListWidget::RefreshFriendList()
{
	if (FriendListContainer)
	{
		FriendListContainer->ClearChildren();
	}

	if (UPGAdvancedFriendsGameInstance* GI = GIRef.Get())
	{
		GI->ReadSteamFriends();
	}
}

void UPGFriendListWidget::OnFriendListUpdated()
{
	if (!FriendListContainer || !FriendEntryWidgetClass)
	{
		return;
	}

	UPGAdvancedFriendsGameInstance* GI = GIRef.Get();
	if (!GI)
	{
		return;
	}

	FriendListContainer->ClearChildren();

	TArray<FSteamFriendInfo> Friends = GI->CachedFriends;
	Friends.Sort([](const FSteamFriendInfo& A, const FSteamFriendInfo& B)
	{
		return A.bIsOnline > B.bIsOnline;
	});

	for (const FSteamFriendInfo& FriendInfo : Friends)
	{
		UPGFriendEntryWidget* NewEntry = CreateWidget<UPGFriendEntryWidget>(this, FriendEntryWidgetClass);
		if (NewEntry)
		{
			NewEntry->SetupFriendEntry(FText::FromString(FriendInfo.DisplayName), FriendInfo.Avatar, FriendInfo.bIsOnline, FriendInfo.NetId);
			FriendListContainer->AddChildToVerticalBox(NewEntry);
		}
	}
}
