// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Player/PGLobbyPlayerController.h"

void UPGGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		SessionInterface = subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPGGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPGGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPGGameInstance::OnJoinSessionComplete);
		}
	}

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));

	// Escape
	ItemDataMap.Add("AdminDevice", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_AdminDevice.DA_Escape_AdminDevice")));
	ItemDataMap.Add("EnergyCore", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_EnergyCore.DA_Escape_EnergyCore")));
	ItemDataMap.Add("RootCalculator", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Escape/DA_Escape_RootCalculator.DA_Escape_RootCalculator")));

}

void UPGGameInstance::HostSession(FName SessionName, int32 MaxPlayers)
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings settings;
		settings.bIsLANMatch = false;
		settings.NumPublicConnections = MaxPlayers;
		settings.bShouldAdvertise = true;
		settings.bUsesPresence = true;
		settings.bAllowJoinInProgress = true;
		settings.bAllowJoinViaPresence = true;
		settings.bIsDedicated = false;
		settings.bUseLobbiesIfAvailable = true;

		SessionInterface->CreateSession(0, SessionName, settings);
	}
}

void UPGGameInstance::FindSessions()
{
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		SessionInterface = subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionSearch = MakeShareable(new FOnlineSessionSearch());
			SessionSearch->bIsLanQuery = false;
			SessionSearch->MaxSearchResults = 10;
			SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);

			SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
		}
	}
}

void UPGGameInstance::JoinSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
	}
}

void UPGGameInstance::SetPlayerName(const FString& NewName)
{
	Playername = NewName;

	APlayerController* pc = GetFirstLocalPlayerController();
	if (pc && pc->PlayerState)
	{
		pc->PlayerState->SetPlayerName(NewName);
	}
}

void UPGGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully"));
		// UGameplayStatics::OpenLevel(GetWorld(), "LV_PGMainLevel", true, "listen");
	}
}

void UPGGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful || !SessionSearch.IsValid()) return;

	APGLobbyPlayerController* pc = Cast<APGLobbyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!pc) return;

	//UPGLobbyUI* lobbyWidget = pc->GetLobbyWidget();
	//if (!lobbyWidget) return;

	//lobbyWidget->ClearSessionList();

	//for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
	//{
	//	const FOnlineSessionSearchResult& result = SessionSearch->SearchResults[i];
	//	const FString& serverName = result.Session.OwningUserName;

	//	lobbyWidget->AddSessionSlot(serverName, i);
	//}
	
	// UE_LOG(LogTemp, Warning, TEXT("No Sessions Found"));
}

void UPGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	FString connectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, connectString))
	{
		APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (pc)
		{
			pc->ClientTravel(connectString, ETravelType::TRAVEL_Absolute);
		}
	}
}

int32 UPGGameInstance::GetMaxInventorySize() const
{
	return MaxInventorySize;
}

UPGItemData* UPGGameInstance::GetItemDataByKey(FName Key)
{
	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		return ptr->LoadSynchronous();
	}
	return nullptr;
}
