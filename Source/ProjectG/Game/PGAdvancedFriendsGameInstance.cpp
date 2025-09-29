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

	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
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
/*
* ���� ����
* ���� ���� ������ �����ִ� ��� DestroySession�� ���� ���� ���� �� ����
*/
void UPGAdvancedFriendsGameInstance::HostSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	const FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(SessionName);
	if (ExistingSession != nullptr)
	{
		bIsHostingAfterDestroy = true;
		PendingSessionName = SessionName;
		PendingMaxPlayers = MaxPlayers;
		bIsPendingSessionPrivate = bIsPrivate;

		SessionInterface->DestroySession(SessionName);
	}
	else
	{
		CreateNewSession(SessionName, MaxPlayers, bIsPrivate);
	}
}

/*
* ���� ���� ������
*/
void UPGAdvancedFriendsGameInstance::CreateNewSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate)
{
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

/*
* ���� ���� �Ϸ� �� ó�� 
*/
void UPGAdvancedFriendsGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		bIsHost = true;
		CurrentSavedGameState = EGameState::Lobby;
		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true, "listen");
	}
	else
	{
		ForceReturnToMainMenu();
	}
}

/*
* ���� �˻�
*/
void UPGAdvancedFriendsGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid())
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

	SessionInterface->FindSessions(0, LatestSessionSearch.ToSharedRef());
}

/*
* ���� �˻� �Ϸ� �� ó��
* ������ ���� �˻� ��� ����
*/
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

/*
* �������� ������ ���� ����
* ������ bUseLobbiesIfAvailable�� bUsesPresence���� �����ؾ� ������ �� ������,
* UE5.3���� ������ bUseLobbiesIfAvailable���� true�� ������ �ȵǴ� ������ �־� �˻��� true�� ���� �ٲ������
*/
void UPGAdvancedFriendsGameInstance::JoinFoundSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !LatestSessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::JoinFoundSession: no valid session interface or session search"));
		return;
	}

	if (LatestSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{		
		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUsesPresence = true;

		SessionInterface->JoinSession(0, NAME_GameSession, LatestSessionSearch->SearchResults[SessionIndex]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::JoinFoundSession: Invalid session index [%d]"), SessionIndex);
	}
}

/*
* ���� ���� �Ϸ� �� ó��
* ȣ��Ʈ �������� �̵�
*/
void UPGAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success && SessionInterface.IsValid())
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			ForceReturnToMainMenu();
		}
	}
	else
	{
		ForceReturnToMainMenu();
	}
}

/*
* ���� �ʴ� ���� �� ó��
* �ʴ� ������ �����Ͽ� ���� ������ ���� �� �ʴ� ���� ���� ����
*/
void UPGAdvancedFriendsGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!bWasSuccessful || !SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnSessionUserInviteAccepted: Failed to accept session invite"));
		return;
	}

	AcceptedInviteInfo = MakeShared<FOnlineSessionSearchResult>(InviteResult);
	LeaveSessionAndReturnToMainMenu();
}

/*
* OSS ���� �Ұ����� ���(Network error) or ������ �ʿ䰡 ���� ���
* GI �ʱ�ȭ �� DestroySession ���� MainMenu�� �̵�
*/
void UPGAdvancedFriendsGameInstance::ForceReturnToMainMenu()
{
	bIsHost = false;
	CurrentSavedGameState = EGameState::MainMenu;

	bIsHostingAfterDestroy = false;
	AcceptedInviteInfo.Reset();
	PendingSessionName = NAME_None;
	PendingMaxPlayers = 0;
	bIsPendingSessionPrivate = false;

	UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
}

/*
* OSS SessionDestroy�� ���� �����ϰ� ���� ���� �� MainMenu�� �̵�
* ȣ��Ʈ�� ��� ���� �� Ŭ���̾�Ʈ���� ������ �� ����
*/
void UPGAdvancedFriendsGameInstance::LeaveSessionAndReturnToMainMenu()
{
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem || !SessionInterface.IsValid())
	{
		ForceReturnToMainMenu();
		return;
	}

	if (bIsHost)
	{
		if (UWorld* World = GetWorld())
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();
				if (PC && !PC->IsLocalController())
				{
					if (APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(PC))
					{
						LobbyPC->Client_ForceReturnToLobby();
					}
					else if (APGPlayerController* PGPC = Cast<APGPlayerController>(PC))
					{
						PGPC->Client_ForceReturnToLobby();
					}
				}
			}
		}
	}

	SessionInterface->DestroySession(NAME_GameSession);
}

/*
* ���� ���� �� ó��
* ����� �÷��� ���¿� ���� �б�
* 1. bIsHostingAfterDestroy (���� �ı� �� ȣ����)
*	 ����� ȣ���� ���� ������ ����Ͽ� ���� ����
*	 ����� �ӽ� ���� �ʱ�ȭ
* 2. AcceptedInviteInfo.IsValid() (���� �ı� �� �ʴ� ���� ���� ����)
*	 ����� �ʴ� ������ ����Ͽ� ���� ����
*    ����� �ӽ� ���� �ʱ�ȭ
* 3. �÷��װ� ���� ���
*	 ������ ������ �����ų� ���� �ı��� ������ ��Ȳ 
*    GameInstance �ʱ�ȭ �� ���� �޴��� �̵�
*/
void UPGAdvancedFriendsGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful && SessionInterface.IsValid())
	{
		if (bIsHostingAfterDestroy)
		{
			CreateNewSession(PendingSessionName, PendingMaxPlayers, bIsPendingSessionPrivate);

			bIsHostingAfterDestroy = false;
			PendingSessionName = NAME_None;
			PendingMaxPlayers = 0;
			bIsPendingSessionPrivate = false;

			return;
		}
		else if (AcceptedInviteInfo.IsValid())
		{
			SessionInterface->JoinSession(0, NAME_GameSession, *AcceptedInviteInfo.Get());

			AcceptedInviteInfo.Reset();

			return;
		}
	}

	ForceReturnToMainMenu();
}

/*
* Travel Failure ó��
* GameInstance �ʱ�ȭ �� ���� �޴��� �̵�
*/
void UPGAdvancedFriendsGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	ForceReturnToMainMenu();
}

/*
* Network Failure ó��
* GameInstance �ʱ�ȭ �� ���� �޴��� �̵�
*/
void UPGAdvancedFriendsGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	ForceReturnToMainMenu();
}

/*
* ������ �˻� �ȵǵ��� ������Ʈ
*/
void UPGAdvancedFriendsGameInstance::CloseSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session)
	{
		UE_LOG(LogTemp, Log, TEXT("GI::CloseSession: Close session for new players."));
		FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
		UpdatedSettings.bShouldAdvertise = false;
		SessionInterface->UpdateSession(NAME_GameSession, UpdatedSettings);
	}
}

/*
* ������ �˻� �����ϵ��� ������Ʈ
*/
void UPGAdvancedFriendsGameInstance::OpenSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session)
	{
		UE_LOG(LogTemp, Log, TEXT("GI::OpenSession: Re-opening session for new players."));
		FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
		UpdatedSettings.bShouldAdvertise = true;
		SessionInterface->UpdateSession(NAME_GameSession, UpdatedSettings);
	}
}

void UPGAdvancedFriendsGameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Log, TEXT("GI::OnUpdateSessionComplete: Session '%s' update completed. Success: %d. bShouldAdvertise: %d"),
			*SessionName.ToString(), bWasSuccessful, SessionInterface->GetNamedSession(NAME_GameSession)->SessionSettings.bShouldAdvertise);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::OnUpdateSessionComplete: Session '%s' update completed but session no longer exists."), *SessionName.ToString());
	}
}

/*
* Travel �� ���� ���� �÷��̾� ���
*/
void UPGAdvancedFriendsGameInstance::SetExpectedPlayersForTravel(const TArray<TObjectPtr<APlayerState>>& InPlayerArray)
{
	ExpectedPlayersForTravel.Empty();
	for (APlayerState* PS : InPlayerArray)
	{
		if (PS)
		{
			ExpectedPlayersForTravel.Add(PS->GetUniqueId());
		}
	}
}

/*
* ���� ���� �÷��̾� �迭 �ʱ�ȭ
*/
void UPGAdvancedFriendsGameInstance::ClearExpectedPlayersForTravel()
{
	ExpectedPlayersForTravel.Empty();
}

/*
* ���ǿ��� �÷��̾� �߹�
*/
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

/*
* ������ ������ �ε�
*/
void UPGAdvancedFriendsGameInstance::RequestLoadItemData(FName Key, FOnItemDataLoaded OnLoadedDelegate)
{
	if (!OnLoadedDelegate.IsBound())
	{
		return;
	}

	if (TSoftObjectPtr<UPGItemData>* ItemDataPtr = ItemDataMap.Find(Key))
	{
		if (ItemDataPtr->IsValid())
		{
			OnLoadedDelegate.Execute(ItemDataPtr->Get());
			return;
		}

		StreamableManager.RequestAsyncLoad(ItemDataPtr->ToSoftObjectPath(), FStreamableDelegate::CreateLambda([OnLoadedDelegate, ItemDataPtr]()
		{		
			if (OnLoadedDelegate.IsBound())
			{
				OnLoadedDelegate.Execute(ItemDataPtr->Get());
			}
		}));
	}
	else
	{
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

/*
* Steam ������ �÷��̾� �ƹ�Ÿ �̹��� �ȼ� ������(RGBA �迭) ��û
* �޾ƿ� �ȼ� �����͸� ����� UTexture2D ����
*/
UTexture2D* UPGAdvancedFriendsGameInstance::GetSteamAvatarAsTexture(const FUniqueNetId& InUserId)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	ISteamFriends* SteamFriendsPtr = SteamFriends();
	ISteamUtils* SteamUtilsPtr = SteamUtils();
	if (!SteamFriendsPtr || !SteamUtilsPtr)
	{
		return nullptr;
	}

	uint64 SteamId64 = *(uint64*)InUserId.GetBytes();
	CSteamID SteamId(SteamId64);
	const int AvatarHandle = SteamFriendsPtr->GetMediumFriendAvatar(SteamId);
	if (AvatarHandle == 0)
	{
		return nullptr;
	}

	uint32 Width, Height;
	if (!SteamUtilsPtr->GetImageSize(AvatarHandle, &Width, &Height))
	{
		return nullptr;
	}

	TArray<uint8> AvatarRGBA;
	AvatarRGBA.SetNum(Width * Height * 4);
	if (!SteamUtilsPtr->GetImageRGBA(AvatarHandle, AvatarRGBA.GetData(), AvatarRGBA.Num()))
	{
		return nullptr;
	}

	UTexture2D* AvatarTexture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
	if (AvatarTexture)
	{
		void* TextureData = AvatarTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, AvatarRGBA.GetData(), AvatarRGBA.Num());
		AvatarTexture->GetPlatformData()->Mips[0].BulkData.Unlock();
		AvatarTexture->UpdateResource();
		return AvatarTexture;
	}
#endif
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
