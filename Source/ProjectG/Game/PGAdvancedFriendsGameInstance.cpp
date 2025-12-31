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
#include "Blueprint/UserWidget.h"

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
	ItemDataMap.Add("ReviveKit", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/ReviveKit/DA_Consumable_ReviveKit.DA_Consumable_ReviveKit")));

	// Escape
	ItemDataMap.Add("ChainKey", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/ChainKey/DA_Exit_ChainKey.DA_Exit_ChainKey")));
	ItemDataMap.Add("HandWheel", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/HandWheel/DA_Exit_HandWheel.DA_Exit_HandWheel")));
	ItemDataMap.Add("RustOil", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/WheelOil/DA_Exit_RustOil.DA_Exit_RustOil")));
}

// ---------- Session ---------
/*
* 세션 생성
* 이전 세션 정보가 남아있는 경우 DestroySession을 통해 세션 종료 후 생성
*/
void UPGAdvancedFriendsGameInstance::HostSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate)
{
	if (!SessionInterface.IsValid())
	{
		OnHostSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Online Subsystem is not available")));
		return;
	}

	OnHostSessionAttemptStarted.Broadcast();

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
* 세션 생성 구현부
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
	SessionSettings.Set(SESSION_KEY_CURRENT_PLAYERS, 1, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->CreateSession(0, SessionName, SessionSettings);
}

/*
* 세션 생성 완료 후 처리 
*/
void UPGAdvancedFriendsGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		bIsHost = true;
		CurrentSavedGameState = EGameState::Lobby;

		OnHostSessionAttemptFinished.Broadcast(true, FText::GetEmpty());

		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true, "listen");
	}
	else
	{
		OnHostSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Failed to create session")));
		//ForceReturnToMainMenu();
	}
}

/*
* 세션 검색
*/
void UPGAdvancedFriendsGameInstance::FindSessions()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::FindSessions: no valid session interface"));
		OnSessionsFound.Broadcast(TArray<FOnlineSessionSearchResult>());
		OnFindSessionAttemptFinished.Broadcast(false);
		return;
	}

	OnFindSessionAttemptStarted.Broadcast();

	LatestSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LatestSessionSearch->bIsLanQuery = false;
	LatestSessionSearch->MaxSearchResults = 20;
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("GAMENAME")), FString(TEXT("ProjectG")), EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, LatestSessionSearch.ToSharedRef());
}

/*
* 세션 검색 완료 후 처리
* 위젯에 세션 검색 목록 전달
*/
void UPGAdvancedFriendsGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	OnFindSessionAttemptFinished.Broadcast(bWasSuccessful);

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
* 위젯에서 선택한 세션 참가
* 세션의 bUseLobbiesIfAvailable과 bUsesPresence값이 동일해야 참가할 수 있지만,
* UE5.3에서 세션의 bUseLobbiesIfAvailable값이 true로 설정이 안되는 현상이 있어 검색후 true로 직접 바꿔줘야함
*/
void UPGAdvancedFriendsGameInstance::JoinFoundSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !LatestSessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::JoinFoundSession: no valid session interface or session search"));
		OnJoinSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Session system error")));
		return;
	}

	if (LatestSessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		OnJoinSessionAttemptStarted.Broadcast();

		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUseLobbiesIfAvailable = true;
		LatestSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.bUsesPresence = true;

		SessionInterface->JoinSession(0, NAME_GameSession, LatestSessionSearch->SearchResults[SessionIndex]);
	}
	else
	{
		OnJoinSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Invalid session selected")));
		UE_LOG(LogTemp, Warning, TEXT("GI::JoinFoundSession: Invalid session index [%d]"), SessionIndex);
	}
}

/*
* 세션 참가 완료 후 처리
* 호스트 세션으로 이동
*/
void UPGAdvancedFriendsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
	{
		OnJoinSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Online Subsystem is not available")));
		return;
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			OnJoinSessionAttemptFinished.Broadcast(true, FText::GetEmpty());

			APlayerController* PC = GetFirstLocalPlayerController();
			if (PC)
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			OnJoinSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Could not resolve connection string")));
			//ForceReturnToMainMenu();
		}
	}
	else
	{
		OnJoinSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Failed to join session")));
		//ForceReturnToMainMenu();
	}
}

/*
* 세션 초대 수락 후 처리
* 초대 정보를 저장하여 현재 세션을 나간 뒤 초대 받은 세션 입장
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
* OSS 접근 불가능한 경우(Network error) or 접근할 필요가 없는 경우
* GI 초기화 후 DestroySession 없이 MainMenu로 이동
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
* OSS SessionDestroy를 통해 안전하게 세션 종료 후 MainMenu로 이동
* 호스트의 경우 세션 내 클라이언트들을 내보낸 후 퇴장
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
* 세션 종료 후 처리
* 저장된 플래그 상태에 따라 분기
* 1. bIsHostingAfterDestroy (세션 파괴 후 호스팅)
*	 저장된 호스팅 세션 정보를 사용하여 세션 생성
*	 사용한 임시 변수 초기화
* 2. AcceptedInviteInfo.IsValid() (세션 파괴 후 초대 받은 세션 참가)
*	 저장된 초대 정보를 사용하여 세션 참가
*    사용한 임시 변수 초기화
* 3. 플래그가 없는 경우
*	 실제로 세션을 나가거나 세션 파괴에 실패한 상황 
*    GameInstance 초기화 후 메인 메뉴로 이동
*/
void UPGAdvancedFriendsGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		OnHostSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Online Subsystem is not available")));
		return;
	}

	if (bWasSuccessful)
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
	else
	{
		if (bIsHostingAfterDestroy)
		{
			OnHostSessionAttemptFinished.Broadcast(false, FText::FromString(TEXT("Failed to destroy previous session")));

			bIsHostingAfterDestroy = false;
			PendingSessionName = NAME_None;
			PendingMaxPlayers = 0;
			bIsPendingSessionPrivate = false;
		}
	}

	ForceReturnToMainMenu();
}

/*
* Travel Failure 처리
* GameInstance 초기화 후 메인 메뉴로 이동
*/
void UPGAdvancedFriendsGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	ForceReturnToMainMenu();
}

/*
* Network Failure 처리
* GameInstance 초기화 후 메인 메뉴로 이동
*/
void UPGAdvancedFriendsGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	ForceReturnToMainMenu();
}

/*
* 세션이 검색 안되도록 업데이트
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
* 세션이 검색 가능하도록 업데이트
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
* Travel 전 현재 세션 플레이어 기록
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
* 현재 세션 플레이어 배열 초기화
*/
void UPGAdvancedFriendsGameInstance::ClearExpectedPlayersForTravel()
{
	ExpectedPlayersForTravel.Empty();
}

/*
* 세션에서 플레이어 추방
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

void UPGAdvancedFriendsGameInstance::UpdateSessionPlayerCount(int32 CurrentPlayers)
{
	if (!bIsHost || !SessionInterface.IsValid())
	{
		return;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (!Session)
	{
		return;
	}

	FOnlineSessionSettings UpdatedSettings = Session->SessionSettings;
	UpdatedSettings.Set(SESSION_KEY_CURRENT_PLAYERS, CurrentPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
	UE_LOG(LogTemp, Log, TEXT("GI::UpdateSessionPlayerCount: Updating session player count to %d"), CurrentPlayers);
	SessionInterface->UpdateSession(NAME_GameSession, UpdatedSettings, true);
}
// ---------- Session ---------

void UPGAdvancedFriendsGameInstance::ShowLoadingScreen()
{
	if (!LoadingScreenWidgetClass || (LoadingScreenWidget && LoadingScreenWidget->IsInViewport()))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GI::ShowLoadingScreen: Show loading screen"));
	LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
	if (!LoadingScreenWidget)
	{
		return;
	}
	LoadingScreenWidget->AddToViewport(100);
}

void UPGAdvancedFriendsGameInstance::HideLoadingScreen()
{
	if (!LoadingScreenWidget)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GI::HideLoadingScreen: hide loading screen"));
	LoadingScreenWidget->RemoveFromParent();
	LoadingScreenWidget = nullptr;
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

/*
* 아이템 데이터 로드
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
* Steam 서버에 플레이어 아바타 이미지 픽셀 데이터(RGBA 배열) 요청
* 받아온 픽셀 데이터를 사용해 UTexture2D 생성
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
