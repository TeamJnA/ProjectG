// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"

#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerController.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"

#include "Item/PGItemData.h"
#include "UI/PGMainMenuWidget.h"

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include "steam/steam_api.h" 
#endif

void UPGAdvancedFriendsGameInstance::Init()
{
	Super::Init();
	
	if (GEngine)
	{
		GEngine->OnTravelFailure().AddUObject(this, &UPGAdvancedFriendsGameInstance::HandleTravelFailure);
		GEngine->OnNetworkFailure().AddUObject(this, &UPGAdvancedFriendsGameInstance::HandleNetworkFailure);
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnDestroySessionComplete);
			SessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnUpdateSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnSessionUserInviteAccepted);
		}
	}
	
	// gamestate initiate
	CurrentSavedGameState = EGameState::MainMenu;

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));
	ItemDataMap.Add("Key", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/Key/DA_Consumable_Key.DA_Consumable_Key")));

	// Escape
	ItemDataMap.Add("ExitKey", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/ExitKey/DA_Consumable_ExitKey.DA_Consumable_ExitKey")));
}

// ---------- Session ---------
void UPGAdvancedFriendsGameInstance::HostSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::HostSession: HostSession failed. SessionInterface is not valid"));
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(SessionName);
	if (ExistingSession != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Found an existing session '%s'. Destroying it first."), *SessionName.ToString());

		// 1. �ı� �� ������ �ٽ� ������ �Ѵٰ� �÷��׸� �����ϰ�, �Ķ���͸� ����
		bIsHostingAfterDestroy = true;
		PendingSessionName = SessionName;
		PendingMaxPlayers = MaxPlayers;
		bIsPendingSessionPrivate = bIsPrivate;

		// 2. ���� �ı��� ��û
		// �Ϸ�Ǹ� OnDestroySessionComplete�� CreateNewSession ȣ��
		SessionInterface->DestroySession(SessionName);
	}
	else
	{
		// 3. ���� ������ ������ �ٷ� ����
		CreateNewSession(SessionName, MaxPlayers, bIsPrivate);
	}
}

void UPGAdvancedFriendsGameInstance::CreateNewSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate)
{
	TSharedPtr<const FUniqueNetId> UserId = GetFirstGamePlayer()->GetPreferredUniqueNetId().GetUniqueNetId();
	if (!UserId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateNewSession failed, Cannot get a valid user ID."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GI::CreateNewSession: Storing PendingHostId: %s"), *UserId->ToString());

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.NumPrivateConnections = bIsPrivate ? 1 : 0;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowInvites = true;

	SessionSettings.Set(FName(TEXT("GAMENAME")), FString(TEXT("ProjectG")), EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

void UPGAdvancedFriendsGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("GI::OnCreateSessionComplete: Session [%s] created succesfully"), *SessionName.ToString());
		bIsHost = true;
		CurrentSavedGameState = EGameState::Lobby;
		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true, "listen");
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnCreateSessionComplete: Failed create session"));
	}
}

void UPGAdvancedFriendsGameInstance::FindSessions()
{
	ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("GI::FindSessions: no valid local player"));
		OnSessionsFound.Broadcast(TArray<FOnlineSessionSearchResult>());
		return;
	}
	TSharedPtr<const FUniqueNetId> UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();

	if (!SessionInterface.IsValid() || !UserId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::FindSessions: no valid session interface"));
		OnSessionsFound.Broadcast(TArray<FOnlineSessionSearchResult>());
		return;
	}

	LatestSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LatestSessionSearch->bIsLanQuery = false;
	LatestSessionSearch->MaxSearchResults = 20;
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("GAMENAME")), FString(TEXT("ProjectG")), EOnlineComparisonOp::Equals);

	UE_LOG(LogTemp, Log, TEXT("GI::FindSessions: Finding sessions as User: [%s]"), *UserId->ToString());
	SessionInterface->FindSessions(0, LatestSessionSearch.ToSharedRef());
}

void UPGAdvancedFriendsGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (bWasSuccessful && LatestSessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("GI::OnFindSessionsComplete: Found %d sessions."), LatestSessionSearch->SearchResults.Num());
		OnSessionsFound.Broadcast(LatestSessionSearch->SearchResults);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnFindSessionsComplete: FindSessions call failed. bWasSuccessful: %s"), bWasSuccessful ? TEXT("true") : TEXT("false"));
		OnSessionsFound.Broadcast(TArray<FOnlineSessionSearchResult>());
	}
}

void UPGAdvancedFriendsGameInstance::JoinFoundSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !LatestSessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::JoinFoundSession: no valid session interface or session search"));
		return;
	}

	if (LatestSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session at index [%d]"), SessionIndex);
		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session at [%s]"), *LatestSessionSearch->SearchResults[SessionIndex].GetSessionIdStr());
		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session bUseLobbiesIfAvailable [%d]"), LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable);
		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session bUsesPresence [%d]"), LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUsesPresence);
		
		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUsesPresence = true;

		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session bUseLobbiesIfAvailable [%d]"), LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable);
		UE_LOG(LogTemp, Log, TEXT("GI::JoinFoundSession: Joining session bUsesPresence [%d]"), LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUsesPresence);

		SessionInterface->JoinSession(0, NAME_GameSession, LatestSessionSearch->SearchResults[SessionIndex]);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GI::JoinFoundSession: Invalid session index [%d]"), SessionIndex);
	}
}

void UPGAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success && SessionInterface.IsValid())
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			UE_LOG(LogTemp, Log, TEXT("GI::OnJoinSessionComplete: Join session success. Traveling to %s"), *ConnectString);
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::OnJoinSessionComplete: Failed to join session"));
		if (!Result == EOnJoinSessionCompleteResult::Success)
		{
			UE_LOG(LogTemp, Warning, TEXT("GI::OnJoinSessionComplete: Failed"));
		}
		if (!SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("GI::OnJoinSessionComplete: session interface Failed"));

		}
	}
}

void UPGAdvancedFriendsGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnSessionUserInviteAccepted: Failed to accept session invite"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("GI::OnSessionUserInviteAccepted: Invite accepted."));

	// �ʴ� ���� ����
	AcceptedInviteInfo = MakeShared<FOnlineSessionSearchResult>(InviteResult);
	// ���� ���� ������
	LeaveSessionAndReturnToLobby();
}

void UPGAdvancedFriendsGameInstance::LeaveSessionAndReturnToLobby()
{
	UE_LOG(LogTemp, Log, TEXT("GI::LeaveSessionAndReturnToLobby: Attempting to leave current game session."));

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		// �¶��� ����ý����� ã�� �� ������ ��� �κ�� �̵�
		UE_LOG(LogTemp, Warning, TEXT("GI::LeaveSessionAndReturnToLobby: Online Subsystem not found. Forcing return to main menu."));
		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
		return;
	}

	// ȣ��Ʈ�� ������ ������ ���, ������ Ŭ���̾�Ʈ�� ���ǿ��� ��������
	if (bIsHost)
	{
		UE_LOG(LogTemp, Log, TEXT("GI::LeaveSessionAndReturnToLobby: Host is leaving session. Notifying all clients"));

		UWorld* World = GetWorld();
		if (World)
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				// PC�� ��ȿ�ϰ� ����(ȣ��Ʈ)�� PC�� �ƴ� ��� -> Ŭ���̾�Ʈ PC�� ���
				if (PC && !PC->IsLocalController())
				{
					if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(PC))
					{
						LobbyPC->Client_ForceReturnToLobby();
					}

					if (APGPlayerController* PGPC = Cast<APGPlayerController>(PC))
					{
						PGPC->Client_ForceReturnToLobby();
					}
				}
			}
		}
	}

	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UPGAdvancedFriendsGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// "�ı� �� ȣ����" �÷��װ� ���� �ִ��� Ȯ��
		if (bIsHostingAfterDestroy)
		{
			// �÷��׸� �����ϰ�, �����ص� �Ķ���ͷ� ���ο� ���� ����
			bIsHostingAfterDestroy = false;
			UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: Previous session destroyed. Creating new session now."));
			CreateNewSession(PendingSessionName, PendingMaxPlayers, bIsPendingSessionPrivate);
			return; // ���⼭ �Լ��� �����Ͽ� �Ʒ��� �κ� ���� ������ Ÿ�� �ʵ���
		}

		// �ʴ� ������ �ִ� ��� (�ʴ밡 �Ǿ� OnDestroySessionComplete�� ȣ��� ���)
		if (AcceptedInviteInfo.IsValid() && SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: DestroySession successful. Now joining the invited session [%s]"), *AcceptedInviteInfo->GetSessionIdStr());
			UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: invited session bUseLobbiesIfAvailable [%d]"), AcceptedInviteInfo.Get()->Session.SessionSettings.bUseLobbiesIfAvailable);
			UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: invited session bUsesPresence [%d]"), AcceptedInviteInfo.Get()->Session.SessionSettings.bUsesPresence);

			SessionInterface->JoinSession(0, NAME_GameSession, *AcceptedInviteInfo.Get());

			// ���� �ʴ� ���� �ʱ�ȭ
			AcceptedInviteInfo.Reset();
			// �κ�� ���ư��� �ʰ� ����
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::OnDestroySessionComplete: Failed to destroy session %s"), *SessionName.ToString());
		// ������ ��쵵 �ʴ� ���� �ʱ�ȭ
		// ��� ���� ȣ���� �۾� ���
		bIsHostingAfterDestroy = false;
		AcceptedInviteInfo.Reset();
	}

	// �ʴ븦 ���� DestroySession�� ȣ��� ���� �ƴ� ��� (���� ������ ������ ��Ȳ)
	UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: Session destroy complete, back to lobby"));

	CurrentSavedGameState = EGameState::MainMenu;
	bIsHost = false;

	UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
}
void UPGAdvancedFriendsGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	ReturnToMainMenu(FString::Printf(TEXT("Travel Failure: %s"), *ErrorString));
}

void UPGAdvancedFriendsGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	ReturnToMainMenu(FString::Printf(TEXT("Network Failure: %s"), *ErrorString));
}

void UPGAdvancedFriendsGameInstance::ReturnToMainMenu(const FString& Reason)
{
	UE_LOG(LogTemp, Error, TEXT("Return to main menu. Reason: [%s]"), *Reason);

	AcceptedInviteInfo.Reset();
	LeaveSessionAndReturnToLobby();
	// LeaveSessionAndReturnToLobby -> OnDestroySessionComplete ���� GI �ʱ�ȭ
}

void UPGAdvancedFriendsGameInstance::CloseSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session)
	{
		FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
		UpdatedSettings.bShouldAdvertise = false;
		SessionInterface->UpdateSession(NAME_GameSession, UpdatedSettings);
	}
}

void UPGAdvancedFriendsGameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("GI::OnUpdateSessionComplete: Session '%s' update completed. Success: %d. bShouldAdvertise: %d"), 
		*SessionName.ToString(), bWasSuccessful, SessionInterface->GetNamedSession(NAME_GameSession)->SessionSettings.bShouldAdvertise);
}

const TArray<TObjectPtr<APlayerState>>& UPGAdvancedFriendsGameInstance::GetExpectedPlayersForTravel() const
{
	return ExpectedPlayersForTravel;
}

void UPGAdvancedFriendsGameInstance::SetExpectedPlayersForTravel(const TArray<TObjectPtr<APlayerState>>& PlayerArray)
{
	ExpectedPlayersForTravel.Empty();
	for (APlayerState* PS : PlayerArray)
	{
		if (PS)
		{
			ExpectedPlayersForTravel.Add(PS);
		}
	}
}

void UPGAdvancedFriendsGameInstance::ClearExpectedPlayersForTravel()
{
	ExpectedPlayersForTravel.Empty();
}

void UPGAdvancedFriendsGameInstance::KickPlayerFromSession(const FUniqueNetId& PlayerToKickId)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GI::KickPlayerFromSession: Kicking player with ID '%s' from the session."), *PlayerToKickId.ToString());

	SessionInterface->UnregisterPlayer(NAME_GameSession, PlayerToKickId);
}
// ---------- Session ---------

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

int32 UPGAdvancedFriendsGameInstance::GetMaxInventorySize() const
{
	return MaxInventorySize;
}

UPGItemData* UPGAdvancedFriendsGameInstance::GetItemDataByKey(FName Key)
{
	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		return ptr->LoadSynchronous();
	}
	return nullptr;
}

void UPGAdvancedFriendsGameInstance::RequestLoadItemData(FName Key, FOnItemDataLoaded OnLoadedDelegate)
{
	// Delegate�� ���ε��Ǿ����� ������ return
	if (!OnLoadedDelegate.IsBound())
	{
		return;
	}

	if (TSoftObjectPtr<UPGItemData>* ptr = ItemDataMap.Find(Key))
	{
		// �̹� �ε�Ǿ����� Ȯ��
		if (ptr->IsValid())
		{
			OnLoadedDelegate.Execute(ptr->Get());
			return;
		}

		// �񵿱� �ε� ��û
		StreamableManager.RequestAsyncLoad(ptr->ToSoftObjectPath(), FStreamableDelegate::CreateLambda([OnLoadedDelegate, ptr]()
		{		
			// ���ٰ� ȣ��Ǵ� �������� OnLoadedDelegate�� ��ȿ���� ���� �� �����Ƿ� IsBound()�� �ѹ� �� üũ
			if (OnLoadedDelegate.IsBound())
			{
				OnLoadedDelegate.Execute(ptr->Get());
			}
		}));
	}
	else
	{
		// ItemDataMap�� Ű�� ���°��
		OnLoadedDelegate.Execute(nullptr);
	}
}

void UPGAdvancedFriendsGameInstance::ReadSteamFriends()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineFriendsPtr FriendsInterface = OnlineSubsystem->GetFriendsInterface();
		if (FriendsInterface)
		{
			FriendsInterface->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default), FOnReadFriendsListComplete::CreateUObject(this, &UPGAdvancedFriendsGameInstance::OnReadFriendsListComplete));
		}
	}
}

void UPGAdvancedFriendsGameInstance::OnReadFriendsListComplete(int32 LocalUserName, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
		if (OnlineSubsystem)
		{
			IOnlineFriendsPtr FriendsInterface = OnlineSubsystem->GetFriendsInterface();
			if (FriendsInterface.IsValid())
			{
				CachedFriends.Empty();

				TArray<TSharedRef<FOnlineFriend>> Friends;
				FriendsInterface->GetFriendsList(0, ListName, Friends);

				for (const TSharedRef<FOnlineFriend>& Friend : Friends)
				{
					FSteamFriendInfo FriendInfo;
					FriendInfo.DisplayName = Friend->GetDisplayName();
					FriendInfo.NetId = Friend->GetUserId();
					FriendInfo.Avatar = GetSteamAvatarAsTexture(*Friend->GetUserId());

					if (FriendInfo.NetId.IsValid())
					{
						uint64 SteamId64 = *(uint64*)FriendInfo.NetId->GetBytes();
						CSteamID SteamId(SteamId64);

						EPersonaState PersonaState = SteamFriends()->GetFriendPersonaState(SteamId);
						FriendInfo.bIsOnline = (PersonaState != k_EPersonaStateOffline);
					}
					else
					{
						FriendInfo.bIsOnline = false;
					}

					UE_LOG(LogTemp, Warning, TEXT("Friend: %s, IsOnline: %s"), *FriendInfo.DisplayName, FriendInfo.bIsOnline ? TEXT("True") : TEXT("False"));

					CachedFriends.Add(FriendInfo);
				}

				OnFriendListUpdated.Broadcast();
			}
		}
	}
}

bool UPGAdvancedFriendsGameInstance::GetSteamAvatarAsRawData(const FUniqueNetId& InUserId, TArray<uint8>& OutRawData, int32& OutWidth, int32& OutHeight)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	// �������� ��� ���� ������ �ȵż� ���� ������ �ڵ�
	ISteamFriends* SteamFriendsPtr = SteamFriends();
	ISteamUtils* SteamUtilsPtr = SteamUtils();

	if (!SteamFriendsPtr || !SteamUtilsPtr)
	{
		return false;
	}

	uint64 SteamId64 = *(uint64*)InUserId.GetBytes();
	CSteamID SteamId(SteamId64);

	// �߰� ũ�� �ƹ�Ÿ �ڵ��� ��������
	int AvatarHandle = SteamFriendsPtr->GetMediumFriendAvatar(SteamId);
	if (AvatarHandle == 0)
	{
		return false;
	}

	uint32 Width, Height;
	if (!SteamUtilsPtr->GetImageSize(AvatarHandle, &Width, &Height))
	{
		return false;
	}

	OutRawData.SetNum(Width * Height * 4);
	if (!SteamUtilsPtr->GetImageRGBA(AvatarHandle, OutRawData.GetData(), OutRawData.Num()))
	{
		return false;
	}

	OutWidth = Width;
	OutHeight = Height;
	return true;
#else
	return false;
#endif
}

UTexture2D* UPGAdvancedFriendsGameInstance::GetSteamAvatarAsTexture(const FUniqueNetId& InUserId)
{
	TArray<uint8> AvatarRGBA;
	int32 Width, Height;

	if (GetSteamAvatarAsRawData(InUserId, AvatarRGBA, Width, Height))
	{
		UTexture2D* AvatarTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
		if (AvatarTexture)
		{
			void* TextureData = AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, AvatarRGBA.GetData(), AvatarRGBA.Num());
			AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
			AvatarTexture->UpdateResource();
			return AvatarTexture;
		}
	}

	return nullptr;
}

void UPGAdvancedFriendsGameInstance::InviteFriend(const FUniqueNetId& FriendToInvite)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterfaceRef = OnlineSubsystem->GetSessionInterface();
		if (SessionInterfaceRef.IsValid())
		{
			SessionInterfaceRef->SendSessionInviteToFriend(0, NAME_GameSession, FriendToInvite);
		}
	}
}
