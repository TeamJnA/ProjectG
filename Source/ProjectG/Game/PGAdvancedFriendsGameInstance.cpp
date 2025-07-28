// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"

#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Item/PGItemData.h"

#include "UI/PGMainMenuWidget.h"

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
	
	// gamestate initiate
	CurrentSavedGameState = EGameState::MainMenu;

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));
	ItemDataMap.Add("Key", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/Key/DA_Consumable_Key.DA_Consumable_Key")));

	// Escape
	ItemDataMap.Add("ExitKey", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/ExitKey/DA_Consumable_ExitKey.DA_Consumable_ExitKey")));
}

void UPGAdvancedFriendsGameInstance::HandleOnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("GI::HandleOnCreateSessionComplete: C++ HandleOnlineCreateSessionComplete called."));

	CurrentSavedGameState = EGameState::Lobby;
	bIsHost = true;
}

void UPGAdvancedFriendsGameInstance::HandleOnJoinSessionComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("GI::HandleOnJoinSessionComplete: C++ HandleOnJoinSessionComplete called."));
	OnJoinSessionBPComplete.Broadcast(bWasSuccessful);
}

void UPGAdvancedFriendsGameInstance::SaveGameStateOnTravel(EGameState StateToSave)
{
	CurrentSavedGameState = StateToSave;
	//UE_LOG(LogTemp, Log, TEXT("GI::SaveGameStateOnTravel: CurrentSavedGameState: %s"), *UEnum::GetValueAsString(TEXT("EGameState"), CurrentSavedGameState));
}

EGameState UPGAdvancedFriendsGameInstance::LoadGameStateOnTravel()
{
	//UE_LOG(LogTemp, Log, TEXT("GI::LoadGameStateOnTravel: CurrentSavedGameState: %s"), *UEnum::GetValueAsString(TEXT("EGameState"), CurrentSavedGameState));
	return CurrentSavedGameState;
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
	GetWorld()->GetTimerManager().ClearTimer(TravelTimerHandle);

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
		UE_LOG(LogTemp, Error, TEXT("GameInstance::HandleTravelFailure: Travel Failed: %s"), *ErrorString);
		NotifyTravelFailed();
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
		UE_LOG(LogTemp, Warning, TEXT("GameInstance::InitiateTravelTimer: Start check client travel timeout. TravelRetryCount: %d"), TravelRetryCount);
		if (!GetWorld())
		{
			bDidRetryClientTravel = false;
			bTimeoutProcessInProgress = false;
			bOnTravelFailureDetected = false;
			TravelRetryCount = 0;
			LeaveSessionAndReturnToLobby();
			return;
		}
		// for retry test
		//GetWorld()->GetTimerManager().SetTimer(TravelTimerHandle, this, &UPGAdvancedFriendsGameInstance::OnTravelTimeout, 0.1f, false);
		GetWorld()->GetTimerManager().SetTimer(TravelTimerHandle, this, &UPGAdvancedFriendsGameInstance::OnTravelTimeout, 2.0f, false);

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
	if (!GetWorld()->GetTimerManager().IsTimerActive(TravelTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("GameInstance::NotifyTravelSuccess: Already travel timeout"));
		return;
	}
	if (!GetWorld())
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GameInstance::NotifyTravelSuccess: Check client travel success"));
	GetWorld()->GetTimerManager().ClearTimer(TravelTimerHandle);
	bDidRetryClientTravel = false;
	bTimeoutProcessInProgress = false;
	bOnTravelFailureDetected = false;
	TravelRetryCount = 0;
}

/*
* Check timer is active (debug)
* if timer is active -> travel success
* if timer is not active -> travel timeout
*/
bool UPGAdvancedFriendsGameInstance::CheckIsTimerActive()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TravelTimerHandle))
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		ensure(PC);

		float remainTime = GetTimerManager().GetTimerRemaining(TravelTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("GI::CheckIsTimerActive: [%s] [%s] Timer is active. Remain %.2f s."), *PC->GetName(), *GetNameSafe(this), remainTime);
		return true;
	}
	else
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		ensure(PC);

		UE_LOG(LogTemp, Warning, TEXT("GI::CheckIsTimerActive: [%s] [%s] Timer is not active"), *PC->GetName(), *GetNameSafe(this));
		return false;
	}
}

void UPGAdvancedFriendsGameInstance::OnTravelTimeout()
{
	if (bOnTravelFailureDetected) return;
	bTimeoutProcessInProgress = true;
	GetWorld()->GetTimerManager().ClearTimer(TravelTimerHandle);

	UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: client travel timeout"));
	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: no world"));
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
		UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: no PC"));
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;
		LeaveSessionAndReturnToLobby();
		return;
	}

	// prevent RetryTravel double call
	if (bOnTravelFailureDetected) return;
	UE_LOG(LogTemp, Error, TEXT("GameInstance::OnTravelTimeout: [%s] travel timeout!"), *PC->GetName());
	NotifyTravelFailed();
}

/*
* retry ServerTravel
* if retry count >= Max retry count(2) -> leave session
*/
void UPGAdvancedFriendsGameInstance::NotifyTravelFailed()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		LeaveSessionAndReturnToLobby();
		return;
	}
	APGPlayerController* PC = Cast<APGPlayerController>(world->GetFirstPlayerController());
	if (!PC)
	{
		LeaveSessionAndReturnToLobby();
		return;
	}
	
	UE_LOG(LogTemp, Error, TEXT("GameInstnace::RetryTravel: Retry travel to main level [%s]"), *PC->GetName());
	bDidRetryClientTravel = true;
	TravelRetryCount++;
	bOnTravelFailureDetected = false;
	bTimeoutProcessInProgress = false;
	PC->NotifyTravelFailed();
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
	UE_LOG(LogTemp, Log, TEXT("GI::LeaveSessionAndReturnToLobby: Attempting to leave current game session."));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterfaceRef = OnlineSubsystem->GetSessionInterface();
		if (SessionInterfaceRef.IsValid())
		{
			// ���� �ı� �� ȣ��� ��������Ʈ ���ε�
			// ���� �ڵ��� �ִٸ� ����(�ߺ�����)		
			if (DestroySessionCompleteDelegateHandle.IsValid())
			{
				SessionInterfaceRef->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
			}
			DestroySessionCompleteDelegateHandle = SessionInterfaceRef->AddOnDestroySessionCompleteDelegate_Handle(
				FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPGAdvancedFriendsGameInstance::OnDestroySessionComplete)
			);
			// ���� �÷��̾ ������ ȣ��Ʈ���� Ȯ��
			// �Ϲ������� CreateSession/FindSessions/JoinSession �Լ� ȣ�� �� ���� �̸��� ����
			FName CurrentSessionName = NAME_GameSession;

			UWorld* World = GetWorld();
			// ȣ��Ʈ��� ������ �ı�
			if (World && World->IsNetMode(NM_ListenServer))
			{
				// ������ �ı��ϱ� ���� �÷��̾ ���� ��� ���� (���� ���������� ���� ����)
				// SessionInterface->UnregisterPlayer(CurrentSessionName, GetPrimaryPlayerController()->GetLocalPlayer()->GetPreferredUniqueNetId().ToSharedRef());
				UE_LOG(LogTemp, Log, TEXT("This instance is the host. Destroying session."));

				bool bDestroyed = SessionInterfaceRef->DestroySession(CurrentSessionName);
				if (!bDestroyed)
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to initiate session destruction for %s."), *CurrentSessionName.ToString());
					// ���� �ı��� �������� ��쿡�� �ϴ� ���� �޴��� �̵� �õ�
					UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
				}
			}
			else // Ŭ���̾�Ʈ��� (�Ǵ� ������ �������� �ʰų� �̹� ����� ���¶��)
			{
				// ���� ������ ���� ���� �޴��� �̵��մϴ�.
				UE_LOG(LogTemp, Log, TEXT("Not host or session not active. Returning to main menu."));
				UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
			}
		}
		else
		{
			// �¶��� ����ý����� ���� �������̽��� �������� ���� ��� (��: Null Subsystem)
			UE_LOG(LogTemp, Warning, TEXT("Online Session Interface not valid. Returning to main menu."));
			UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
		}
	}
	else
	{
		// �¶��� ����ý����� ã�� �� ���� ���
		UE_LOG(LogTemp, Warning, TEXT("Online Subsystem not found. Returning to main menu."));
		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
	}
}

void UPGAdvancedFriendsGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterfaceRef = OnlineSubsystem->GetSessionInterface();
		if (SessionInterfaceRef.IsValid())
		{
			SessionInterfaceRef->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;

		CurrentSavedGameState = EGameState::MainMenu;
		bIsHost = false;
		UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: Session %s destroyed successfully"), *SessionName.ToString());
	}
	else
	{
		bDidRetryClientTravel = false;
		bTimeoutProcessInProgress = false;
		bOnTravelFailureDetected = false;
		TravelRetryCount = 0;

		CurrentSavedGameState = EGameState::MainMenu;
		bIsHost = false;
		UE_LOG(LogTemp, Warning, TEXT("GI::OnDestroySessionComplete: Failed to destroy session %s"), *SessionName.ToString());
	}

	UE_LOG(LogTemp, Warning, TEXT("GI::OnDestroySessionComplete: Session destroy complete, back to lobby"));
	UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
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
