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
	
	if (GEngine)
	{
		GEngine->OnTravelFailure().AddUObject(this, &UPGAdvancedFriendsGameInstance::HandleTravelFailure);
	}

	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		SessionInterface = subsystem->GetSessionInterface();

	//	if (SessionInterface.IsValid())
	//	{
	//		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnCreateSessionComplete);
	//		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnFindSessionsComplete);
	//		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnJoinSessionComplete);
	//	}
	//	else
	//	{
	//		UE_LOG(LogTemp, Error, TEXT("SessionInterface invalid"));
	//	}
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem is NULL"));
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

/*
* on notified travel failure
* -> retry travel (Max retry count == 2)
*/
void UPGAdvancedFriendsGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	if (bTimeoutProcessInProgress) return;
	bOnTravelFailureDetected = true;

	if (TravelRetryCount <= 2)
	{
		if (!GetWorld())
		{
			bDidRetryClientTravel = false;
			bTimeoutProcessInProgress = false;
			bOnTravelFailureDetected = false;
			TravelRetryCount = 0;
			LeaveSessionAndReturnToLobby();
			return;
		}
		GetWorld()->GetTimerManager().ClearTimer(TravelTimerHandle);
		UE_LOG(LogTemp, Error, TEXT("GameInstance::HandleTravelFailure: Travel Failed: %s"), *ErrorString);
		RetryClientTravel();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance::HandleTravelFailure: Travel retry failed. LeaveSession"));
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
	}
}

/*
* on non-notified travel failure
* should check manually by timer
*/
void UPGAdvancedFriendsGameInstance::InitiateTravelTimer()
{
	if (TravelRetryCount <= 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance::InitiateTravelTimer: Start check client travel timeout"));
		if (!GetWorld())
		{
			bDidRetryClientTravel = false;
			bTimeoutProcessInProgress = false;
			bOnTravelFailureDetected = false;
			TravelRetryCount = 0;
			LeaveSessionAndReturnToLobby();
			return;
		}
		GetWorld()->GetTimerManager().SetTimer(TravelTimerHandle, this, &UPGAdvancedFriendsGameInstance::OnTravelTimeout, 10.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance::InitiateTravelTimer: Travel retry failed. LeaveSession"));
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
	}
}

/*
* call on travel sueccess
*/
void UPGAdvancedFriendsGameInstance::NotifyTravelSuccess()
{
	UE_LOG(LogTemp, Warning, TEXT("GameInstance::NotifyTravelSuccess: Check client travel success"));
	if (!GetWorld())
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TravelTimerHandle);
	bDidRetryClientTravel = false;
	bTimeoutProcessInProgress = false;
	bOnTravelFailureDetected = false;
	TravelRetryCount = 0;
}

void UPGAdvancedFriendsGameInstance::CheckIsTimerActive()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TravelTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::CheckIsTimerActive: Timer is active"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::CheckIsTimerActive: Timer is not active"));
	}
}

void UPGAdvancedFriendsGameInstance::OnTravelTimeout()
{
	if (bOnTravelFailureDetected) return;
	bTimeoutProcessInProgress = true;

	UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: client travel timeout"));
	UWorld* world = GetWorld();
	if (!world)
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(world, 0);
	if (!PC)
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: [%s] travel timeout!"), *PC->GetName());
	RetryClientTravel();
}

/*
* retry ClientTravel
* if retry count >= Max retry count(2) -> leave session
*/
void UPGAdvancedFriendsGameInstance::RetryClientTravel()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		LeaveSessionAndReturnToLobby();
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(world, 0);
	if (!PC)
	{
		LeaveSessionAndReturnToLobby();
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("GameInstnace::RetryClientTravel: Retry travel to main level [%s]"), *PC->GetName());
	bDidRetryClientTravel = true;
	TravelRetryCount++;
	InitiateTravelTimer();
	bOnTravelFailureDetected = false;
	bTimeoutProcessInProgress = false;
	PC->ClientTravel("/Game/ProjectG/Levels/LV_PGMainLevel", ETravelType::TRAVEL_Absolute);
}

//void UPGAdvancedFriendsGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
//{
//	if (bWasSuccessful)
//	{
//		UE_LOG(LogTemp, Log, TEXT("Session created successfully"));
//
//		//if (GetWorld()->GetNetMode() != NM_Client)
//		//{
//		//	UGameplayStatics::OpenLevel(GetWorld(), "LV_Lobby", true, "listen");
//		//}
//	}
//}
//
//void UPGAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
//{
//	if (!SessionInterface.IsValid()) return;
//
//	FString connectString;
//	if (SessionInterface->GetResolvedConnectString(SessionName, connectString))
//	{
//		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//		if (PC)
//		{
//			PC->ClientTravel(connectString, ETravelType::TRAVEL_Absolute);
//		}
//	}
//}
//
//void UPGAdvancedFriendsGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
//{
//	if (!bWasSuccessful || !SessionSearch.IsValid()) return;
//
//	APGLobbyPlayerController* pc = Cast<APGLobbyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
//	if (!pc) return;
//
//	UPGLobbyWidget* lobbyWidget = pc->GetLobbyWidget();
//	if (!lobbyWidget) return;
//
//	lobbyWidget->ClearSessionList();
//
//	for (int32 i = 0; i < SessionSearch->SearchResults.Num(); ++i)
//	{
//		const FOnlineSessionSearchResult& result = SessionSearch->SearchResults[i];
//		const FString& serverName = result.Session.OwningUserName;
//
//		UE_LOG(LogTemp, Log, TEXT("Session %d: %s"), i, *result.Session.OwningUserName);
//
//		lobbyWidget->AddSessionSlot(serverName, i);
//	}
//}

bool UPGAdvancedFriendsGameInstance::DidRetryClientTravel() const
{
	return bDidRetryClientTravel;
}

void UPGAdvancedFriendsGameInstance::LeaveSessionAndReturnToLobby()
{
	IOnlineSubsystem* subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		IOnlineSessionPtr sessions = subsystem->GetSessionInterface();
		if (sessions.IsValid())
		{
			sessions->DestroySession(NAME_GameSession, FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPGAdvancedFriendsGameInstance::OnDestroySessionComplete));
		}
	}
}

void UPGAdvancedFriendsGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Session destroy complete, back to lobby"));
	UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGMainLevel"), true);
}

int32 UPGAdvancedFriendsGameInstance::GetExpectedPlayerCount()
{
	return ExpectedPlayerCount;
}

void UPGAdvancedFriendsGameInstance::SetExpectedPlayerCount(int32 PlayerCount)
{
	ExpectedPlayerCount = PlayerCount;
}

int32 UPGAdvancedFriendsGameInstance::GetMaxInventorySize() const
{
	return MaxInventorySize;
}

//void UPGAdvancedFriendsGameInstance::HostSession(FName SessionName, int32 MaxPlayers)
//{
//	if (!SessionInterface.IsValid()) return;
//
//	FOnlineSessionSettings sessionSettings;
//	sessionSettings.NumPublicConnections = MaxPlayers;
//
//	sessionSettings.bIsLANMatch = false;
//	sessionSettings.bAllowInvites = true;
//	sessionSettings.bIsDedicated = false;
//	sessionSettings.bUseLobbiesIfAvailable = true;
//	sessionSettings.bUsesPresence = true;
//	sessionSettings.bAllowJoinViaPresence = true;
//	sessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
//	sessionSettings.bAntiCheatProtected = false;
//	sessionSettings.bUsesStats = false;
//	sessionSettings.bShouldAdvertise = true;
//	sessionSettings.bUseLobbiesVoiceChatIfAvailable = false;
//
//	SessionInterface->CreateSession(0, SessionName, sessionSettings);
//}
//
//void UPGAdvancedFriendsGameInstance::JoinSession(int32 SessionIndex)
//{
//	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;
//	if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex)) return;
//
//	SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[SessionIndex]);
//}
//
//void UPGAdvancedFriendsGameInstance::FindSessions()
//{
//	if (!SessionInterface.IsValid()) return;
//
//	SessionSearch = MakeShareable(new FOnlineSessionSearch());
//	SessionSearch->bIsLanQuery = false;
//	SessionSearch->MaxSearchResults = 20;
//	SessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);
//
//	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
//}

UPGItemData* UPGAdvancedFriendsGameInstance::GetItemDataByKey(FName Key)
{
	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		return ptr->LoadSynchronous();
	}
	return nullptr;
}
