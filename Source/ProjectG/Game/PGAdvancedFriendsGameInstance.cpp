// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Player/PGLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Item/PGItemData.h"
#include "UI/PGLobbyWidget.h"

void UPGAdvancedFriendsGameInstance::Init()
{
	Super::Init();
	
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		SessionInterface = subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnJoinSessionComplete);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("SessionInterface invalid"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is NULL"));
	}

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));

	// Escape
	ItemDataMap.Add("AdminDevice", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_AdminDevice.DA_Escape_AdminDevice")));
	ItemDataMap.Add("EnergyCore", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_EnergyCore.DA_Escape_EnergyCore")));
	ItemDataMap.Add("RootCalculator", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_RootCalculator.DA_Escape_RootCalculator")));
}

void UPGAdvancedFriendsGameInstance::SetPlayerName(const FString& NewName)
{
	Playername = NewName;

	APlayerController* pc = GetFirstLocalPlayerController();
	if (pc && pc->PlayerState)
	{
		pc->PlayerState->SetPlayerName(NewName);
	}
}

void UPGAdvancedFriendsGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully"));

		if (GetWorld()->GetNetMode() != NM_Client)
		{
			UGameplayStatics::OpenLevel(GetWorld(), "LV_Lobby", true, "listen");
		}
	}
}

void UPGAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	FString connectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, connectString))
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			PC->ClientTravel(connectString, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UPGAdvancedFriendsGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful || !SessionSearch.IsValid()) return;

	APGLobbyPlayerController* pc = Cast<APGLobbyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!pc) return;

	UPGLobbyWidget* lobbyWidget = pc->GetLobbyWidget();
	if (!lobbyWidget) return;

	lobbyWidget->ClearSessionList();

	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	{
		const FOnlineSessionSearchResult& result = SessionSearch->SearchResults[i];
		const FString& serverName = result.Session.OwningUserName;

		UE_LOG(LogTemp, Log, TEXT("Session %d: %s"), i, *result.Session.OwningUserName);

		lobbyWidget->AddSessionSlot(serverName, i);
	}
}

int32 UPGAdvancedFriendsGameInstance::GetMaxInventorySize() const
{
	return MaxInventorySize;
}

void UPGAdvancedFriendsGameInstance::HostSession(FName SessionName, int32 MaxPlayers)
{
	if (!SessionInterface.IsValid()) return;

	FOnlineSessionSettings sessionSettings;
	sessionSettings.NumPublicConnections = MaxPlayers;

	sessionSettings.bIsLANMatch = false;
	sessionSettings.bAllowInvites = true;
	sessionSettings.bIsDedicated = false;
	sessionSettings.bUseLobbiesIfAvailable = true;
	sessionSettings.bUsesPresence = true;
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	sessionSettings.bAntiCheatProtected = false;
	sessionSettings.bUsesStats = false;
	sessionSettings.bShouldAdvertise = true;
	sessionSettings.bUseLobbiesVoiceChatIfAvailable = false;

	SessionInterface->CreateSession(0, SessionName, sessionSettings);
}

void UPGAdvancedFriendsGameInstance::JoinSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;
	if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex)) return;

	SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
}

void UPGAdvancedFriendsGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid()) return;

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = false;
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

UPGItemData* UPGAdvancedFriendsGameInstance::GetItemDataByKey(FName Key)
{
	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		return ptr->LoadSynchronous();
	}
	return nullptr;
}
