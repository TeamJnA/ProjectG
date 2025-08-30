// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PGFriendListWidget.h"

#include "UI/PGFriendEntryWidget.h"
#include "Components/VerticalBox.h"

#include "Game/PGAdvancedFriendsGameInstance.h"


void UPGFriendListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GIRef = GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (GIRef)
	{
		GIRef->OnFriendListUpdated.AddDynamic(this, &UPGFriendListWidget::OnFriendListUpdated);
	}
}

void UPGFriendListWidget::RefreshFriendList()
{
	if (FriendListContainer)
	{
		FriendListContainer->ClearChildren();
	}

	if (GIRef)
	{
		GIRef->ReadSteamFriends();
	}
}

void UPGFriendListWidget::OnFriendListUpdated()
{
	if (GIRef && FriendListContainer && FriendEntryWidgetClass)
	{
		FriendListContainer->ClearChildren();

		TArray<FSteamFriendInfo> Friends = GIRef->CachedFriends;

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
}
