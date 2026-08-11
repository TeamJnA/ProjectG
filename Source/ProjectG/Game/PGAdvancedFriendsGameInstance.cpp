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
#include "Player/PGGameUserSettings.h"
#include "AudioMixerBlueprintLibrary.h"

#include "Game/PGProgressionSetting.h"
#include "Engine/DataTable.h"

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include "steam/steam_api.h" 
#endif

#define LOCTEXT_NAMESPACE "PGSession"

namespace
{
	const FString ProfileSlotName = TEXT("PGPlayerProfile");
}

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
			SessionInterface->OnSessionUserInviteAcceptedDelegates.RemoveAll(this);

			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnDestroySessionComplete);
			SessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnUpdateSessionComplete);
			SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnSessionUserInviteAccepted);
		}
	}

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UPGAdvancedFriendsGameInstance::OnWorldLoaded);

	// gamestate initiate
	CurrentSavedGameState = EGameState::MainMenu;

	// 저장된 오디오/마이크 설정 적용
	if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
	{
		if (!Settings->LanguageCulture.IsEmpty())
		{
			FInternationalization::Get().SetCurrentLanguageAndLocale(Settings->LanguageCulture);
		}
		Settings->ApplyMicSettings();
	}

	LoadProfile();

	// Consumable
	ItemDataMap.Add("Brick", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick")));
	ItemDataMap.Add("Key", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/Key/DA_Consumable_Key.DA_Consumable_Key")));
	ItemDataMap.Add("ReviveKit", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/ReviveKit/DA_Consumable_ReviveKit.DA_Consumable_ReviveKit")));
	ItemDataMap.Add("Match", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/Match/DA_Consumable_Match.DA_Consumable_Match")));
	ItemDataMap.Add("GlassBottle", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Consumable/GlassBottle/DA_Consumable_GlassBottle.DA_Consumable_GlassBottle")));

	// Escape
	ItemDataMap.Add("ChainKey", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/ChainKey/DA_Exit_ChainKey.DA_Exit_ChainKey")));
	ItemDataMap.Add("HandWheel", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/HandWheel/DA_Exit_HandWheel.DA_Exit_HandWheel")));
	ItemDataMap.Add("RustOil", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/WheelOil/DA_Exit_RustOil.DA_Exit_RustOil")));
	ItemDataMap.Add("Fuse", TSoftObjectPtr<UPGItemData>(FSoftObjectPath("/Game/ProjectG/Items/Exit/Fuse/DA_Exit_Fuse.DA_Exit_Fuse")));
}

// ---------- Session ---------
/*
* 세션 생성
* 이전 세션 정보가 남아있는 경우 DestroySession을 통해 세션 종료 후 생성
*/
void UPGAdvancedFriendsGameInstance::HostSession(const FPGHostSessionOptions& Options)
{
	if (!SessionInterface.IsValid())
	{
		OnHostSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_NoOnlineSubsystem", "Online Subsystem is not available"));
		return;
	}

	SelectedDifficulty = Options.Difficulty;
	OnHostSessionAttemptStarted.Broadcast();

	const FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bIsHostingAfterDestroy = true;
		PendingHostOptions = Options;

		SessionInterface->DestroySession(NAME_GameSession);
	}
	else
	{
		CreateNewSession(Options);
	}
}

/*
* 세션 생성 구현부
*/
void UPGAdvancedFriendsGameInstance::CreateNewSession(const FPGHostSessionOptions& Options)
{
	CurrentHostOptions = Options;

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = Options.bIsSinglePlay ? 1 : PG_MAX_SESSION_PLAYERS;
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bIsLANMatch = false;

	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	if (Options.bIsSinglePlay)
	{
		SessionSettings.bShouldAdvertise = false;
		SessionSettings.bAllowJoinInProgress = false;
		SessionSettings.bAllowInvites = false;
		SessionSettings.bAllowJoinViaPresence = false;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	}
	else
	{
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bAllowJoinInProgress = true;
		SessionSettings.bAllowInvites = true;
		SessionSettings.bAllowJoinViaPresence = !Options.bIsInviteOnly;
		SessionSettings.bAllowJoinViaPresenceFriendsOnly = Options.bIsInviteOnly;
	}

	SessionSettings.Set(FName(TEXT("GAMENAME")), FString(TEXT("ProjectG")), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SESSION_KEY_SESSION_NAME, Options.DisplayName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SESSION_KEY_CURRENT_PLAYERS, 1, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SESSION_KEY_DIFFICULTY, (int32)Options.Difficulty, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SESSION_KEY_INVITE_ONLY, Options.bIsInviteOnly ? 1 : 0, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
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
		OnHostSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_CreateSessionFailed", "Failed to create session"));
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
	
	if (LatestSessionSearch.IsValid() && LatestSessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::FindSessions: Search already in progress"));
		return;
	}

	OnFindSessionAttemptStarted.Broadcast();

	LatestSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LatestSessionSearch->bIsLanQuery = false;
	LatestSessionSearch->MaxSearchResults = 20;
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
	LatestSessionSearch->QuerySettings.Set(FName(TEXT("GAMENAME")), FString(TEXT("ProjectG")), EOnlineComparisonOp::Equals);
	LatestSessionSearch->QuerySettings.Set(SESSION_KEY_INVITE_ONLY, 0, EOnlineComparisonOp::Equals);

	SessionInterface->FindSessions(0, LatestSessionSearch.ToSharedRef());
}

/*
* 세션 검색 완료 후 처리
* 위젯에 세션 검색 목록 전달
*/
void UPGAdvancedFriendsGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	OnFindSessionAttemptFinished.Broadcast(bWasSuccessful);
	VisibleSessionResults.Empty();

	if (bWasSuccessful && LatestSessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : LatestSessionSearch->SearchResults)
		{
			int32 InviteOnlyValue = 0;
			Result.Session.SessionSettings.Get(SESSION_KEY_INVITE_ONLY, InviteOnlyValue);
			if (InviteOnlyValue != 0)
			{
				// FriendsOnly 세션은 목록에 노출x
				UE_LOG(LogTemp, Log, TEXT("GI::OnFindSessionsComplete: Filtered out invite-only session"));
				continue;
			}

			VisibleSessionResults.Add(Result);
		}

		UE_LOG(LogTemp, Log, TEXT("GI::OnFindSessionsComplete: Found %d sessions (%d visible)."),
			LatestSessionSearch->SearchResults.Num(), VisibleSessionResults.Num());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnFindSessionsComplete: FindSessions call failed. bWasSuccessful: %s"),
			bWasSuccessful ? TEXT("true") : TEXT("false"));
	}

	OnSessionsFound.Broadcast(VisibleSessionResults);
}

/*
* Join 전 bUseLobbiesIfAvailable과 bUsesPresence 통일 
*/
void UPGAdvancedFriendsGameInstance::JoinSessionInternal(FOnlineSessionSearchResult& SearchResult)
{
	SearchResult.Session.SessionSettings.bUsesPresence = true;
	SearchResult.Session.SessionSettings.bUseLobbiesIfAvailable = true;

	SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
}

/*
* 위젯에서 선택한 세션 참가
* 세션의 bUseLobbiesIfAvailable과 bUsesPresence값이 동일해야 참가할 수 있지만,
* UE5.3에서 세션의 bUseLobbiesIfAvailable값이 true로 설정이 안되는 현상이 있어 검색후 true로 직접 바꿔줘야함
*/
void UPGAdvancedFriendsGameInstance::JoinFoundSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::JoinFoundSession: no valid session interface or session search"));
		OnJoinSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_SessionSystem", "Session system error"));
		return;
	}

	if (VisibleSessionResults.IsValidIndex(SessionIndex))
	{
		OnJoinSessionAttemptStarted.Broadcast();
		JoinSessionInternal(VisibleSessionResults[SessionIndex]);
	}
	else
	{
		OnJoinSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_InvalidSession", "Invalid session selected"));
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
		OnJoinSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_NoOnlineSubsystem", "Online Subsystem is not available"));
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
			HideLoadingScreen();
			OnJoinSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_ResolveConnectString", "Could not resolve connect string"));
		}
	}
	else
	{
		HideLoadingScreen();
		OnJoinSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_JoinFailed", "Failed to join session"));
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

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GI::OnSessionUserInviteAccepted: Invalid invite result"));
		return;
	}

	AcceptedInviteInfo = MakeShared<FOnlineSessionSearchResult>(InviteResult);

	ShowLoadingScreen();

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		// 기존 세션이 있으면 파괴 후 OnDestroySessionComplete 에서 조인
		LeaveSessionAndReturnToMainMenu();
	}
	else
	{
		// 세션이 없으면(메인메뉴 등) 바로 조인
		bIsHost = false;
		JoinSessionInternal(*AcceptedInviteInfo.Get());
		AcceptedInviteInfo.Reset();
	}
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
	PendingHostOptions = FPGHostSessionOptions();
	CurrentHostOptions = FPGHostSessionOptions();
	VisibleSessionResults.Empty();

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
		OnHostSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_NoOnlineSubsystem", "Online Subsystem is not available"));
		return;
	}

	CurrentHostOptions = FPGHostSessionOptions();

	if (bWasSuccessful)
	{
		if (bIsHostingAfterDestroy)
		{
			CreateNewSession(PendingHostOptions);

			bIsHostingAfterDestroy = false;
			PendingHostOptions = FPGHostSessionOptions();

			return;
		}
		else if (AcceptedInviteInfo.IsValid())
		{
			bIsHost = false;
			JoinSessionInternal(*AcceptedInviteInfo.Get());
			AcceptedInviteInfo.Reset();

			return;
		}
	}
	else
	{
		if (bIsHostingAfterDestroy)
		{
			OnHostSessionAttemptFinished.Broadcast(false, LOCTEXT("Error_DestroyPrevSession", "Failed to destroy previous session"));
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
	UE_LOG(LogTemp, Error, TEXT("GI::HandleTravelFailure: %s"), *ErrorString);

	SetPendingNetworkFailureMessage(LOCTEXT("Error_NetworkError", "Network Error"));

	if (SessionInterface.IsValid())
	{
		if (SessionInterface->GetNamedSession(NAME_GameSession))
		{
			SessionInterface->DestroySession(NAME_GameSession);
			return;
		}
	}

	ForceReturnToMainMenu();
}

/*
* Network Failure 처리
* GameInstance 초기화 후 메인 메뉴로 이동
*/
void UPGAdvancedFriendsGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("GI::HandleNetworkFailure: %s"), *ErrorString);

	FText DisplayMessage = ErrorString.Contains(TEXT("Game Started"))
		? LOCTEXT("Error_GameStarted", "Game Started")
		: LOCTEXT("Error_NetworkError", "Network Error");

	SetPendingNetworkFailureMessage(DisplayMessage);

	if (SessionInterface.IsValid())
	{
		if (SessionInterface->GetNamedSession(NAME_GameSession))
		{
			SessionInterface->DestroySession(NAME_GameSession);
			return;
		}
	}

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
		UpdatedSettings.bAllowJoinInProgress = false;
		UpdatedSettings.bAllowInvites = false;
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
		if (CurrentHostOptions.bIsSinglePlay)
		{
			UpdatedSettings.bShouldAdvertise = false;
			UpdatedSettings.bAllowJoinInProgress = false;
			UpdatedSettings.bAllowInvites = false;
		}
		else
		{
			UpdatedSettings.bShouldAdvertise = true;
			UpdatedSettings.bAllowJoinInProgress = true;
			UpdatedSettings.bAllowInvites = true;
			UpdatedSettings.bAllowJoinViaPresence = !CurrentHostOptions.bIsInviteOnly;
			UpdatedSettings.bAllowJoinViaPresenceFriendsOnly = CurrentHostOptions.bIsInviteOnly;
		}

		SessionInterface->UpdateSession(NAME_GameSession, UpdatedSettings);
	}
}

void UPGAdvancedFriendsGameInstance::OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

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

	if (LoadingScreenWidget)
	{
		LoadingScreenWidget->RemoveFromParent();
		LoadingScreenWidget = nullptr;
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

void UPGAdvancedFriendsGameInstance::SetRemotePlayerVolume(const FUniqueNetIdRepl& PlayerId, float Volume)
{
	if (!PlayerId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] SetRemotePlayerVolume called with invalid PlayerId"));
		return;
	}

	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	RemotePlayerVolumes.Add(PlayerId, ClampedVolume);

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] Set volume for player to %.2f"), ClampedVolume);

}

float UPGAdvancedFriendsGameInstance::GetRemotePlayerVolume(const FUniqueNetIdRepl& PlayerId)
{
	if (!PlayerId.IsValid())
	{
		return 1.0f;
	}

	const float* VolumePtr = RemotePlayerVolumes.Find(PlayerId);
	if (VolumePtr)
	{
		return *VolumePtr;
	}

	return 1.0f;
}

void UPGAdvancedFriendsGameInstance::OnWorldLoaded(UWorld* LoadedWorld)
{
	if (!LoadedWorld)
	{
		return;
	}

	ApplySavedAudioSettings(LoadedWorld);
}

void UPGAdvancedFriendsGameInstance::ApplySavedAudioSettings(UWorld* InWorld)
{
	UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings();
	if (!Settings)
	{
		return;
	}

	if (SoundMixModifier)
	{
		ApplySoundMixOverride(InWorld, SoundClass_Music, Settings->MusicVolume);
		ApplySoundMixOverride(InWorld, SoundClass_SFX, Settings->SFXVolume);
		ApplySoundMixOverride(InWorld, SoundClass_Voice, Settings->VoiceVolume);
	}

	if (!Settings->OutputDeviceId.IsEmpty())
	{
		FOnCompletedDeviceSwap SwapDelegate;
		SwapDelegate.BindDynamic(this, &UPGAdvancedFriendsGameInstance::OnStartupDeviceSwapComplete);
		UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(InWorld, Settings->OutputDeviceId, SwapDelegate);
	}
}

void UPGAdvancedFriendsGameInstance::ApplySoundMixOverride(UWorld* InWorld, USoundClass* InSoundClass, float Volume)
{
	if (!SoundMixModifier || !InSoundClass || !InWorld)
	{
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(InWorld, SoundMixModifier, InSoundClass, Volume, 1.0f, 1.0f, true);
	UGameplayStatics::PushSoundMixModifier(InWorld, SoundMixModifier);
}

void UPGAdvancedFriendsGameInstance::OnStartupDeviceSwapComplete(const FSwapAudioOutputResult& SwapResult)
{
	UE_LOG(LogTemp, Log, TEXT("[Startup] Output device swap - %s, Requested: %s"),
		SwapResult.Result == ESwapAudioOutputDeviceResultState::Success ? TEXT("Success") : TEXT("Failed"),
		*SwapResult.RequestedDeviceId);
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
	if (CurrentHostOptions.bIsSinglePlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::InviteFriend: Ignored. This is a single play session."));
		return;
	}

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

void UPGAdvancedFriendsGameInstance::LoadProfile()
{
	if (UGameplayStatics::DoesSaveGameExist(ProfileSlotName, 0))
	{
		Profile = Cast<UPGSaveGame>(UGameplayStatics::LoadGameFromSlot(ProfileSlotName, 0));
		if (Profile && !Profile->VerifySignature())
		{
			UE_LOG(LogTemp, Warning, TEXT("GI::LoadProfile: signature invalid, resetting."));
			Profile = nullptr;
		}
	}

	if (!Profile)
	{
		Profile = Cast<UPGSaveGame>(UGameplayStatics::CreateSaveGameObject(UPGSaveGame::StaticClass()));
	}
}

void UPGAdvancedFriendsGameInstance::SaveProfile()
{
	if (!Profile)
	{
		return;
	}

	Profile->Signature = Profile->ComputeSignature();
	UGameplayStatics::SaveGameToSlot(Profile, ProfileSlotName, 0);
}

void UPGAdvancedFriendsGameInstance::AddMatchResult(int32 GainedXP)
{
	if (!Profile)
	{
		return;
	}

	PreMatchTotalXP = Profile->TotalXP;
	LastGainedXP = GainedXP;

	Profile->TotalXP += GainedXP;
	Profile->GamesCompleted++;
	Profile->RankIndex = ComputeRankIndex(Profile->TotalXP);
	SaveProfile();

	UE_LOG(LogTemp, Log, TEXT("[Progression] +%d XP, Total:%lld Rank:%d Games:%d"),
		GainedXP, Profile->TotalXP, Profile->RankIndex, Profile->GamesCompleted);
}

UDataTable* UPGAdvancedFriendsGameInstance::GetRankTable()
{
	if (CachedRankTable)
	{
		return CachedRankTable;
	}

	const UPGProgressionSetting* Settings = GetDefault<UPGProgressionSetting>();
	if (!Settings)
	{
		return nullptr;
	}

	CachedRankTable = Settings->RankTable.LoadSynchronous();
	return CachedRankTable;
}

int32 UPGAdvancedFriendsGameInstance::ComputeRankIndex(int64 InTotalXP)
{
	UDataTable* Table = GetRankTable();
	if (!Table)
	{
		return 0;
	}

	TArray<FPGRankRow*> Rows;
	Table->GetAllRows<FPGRankRow>(TEXT("ComputeRankIndex"), Rows);
	Rows.Sort([](const FPGRankRow& A, const FPGRankRow& B)
	{
		return A.RequiredTotalXP < B.RequiredTotalXP;
	});

	int32 Index = 0;
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		if (InTotalXP >= Rows[i]->RequiredTotalXP)
		{
			Index = i;
		}
		else
		{
			break;
		}
	}

	return Index;
}

FPGRankProgress UPGAdvancedFriendsGameInstance::GetRankProgressForXP(int64 InXP)
{
	FPGRankProgress Out;
	UDataTable* Table = GetRankTable();
	if (!Table)
	{
		return Out;
	}

	TArray<FPGRankRow*> Rows;
	Table->GetAllRows<FPGRankRow>(TEXT("GetRankProgressForXP"), Rows);
	if (Rows.Num() == 0)
	{
		return Out;
	}

	Rows.Sort([](const FPGRankRow& A, const FPGRankRow& B)
	{
		return A.RequiredTotalXP < B.RequiredTotalXP;
	});

	int32 Index = 0;
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		if (InXP >= Rows[i]->RequiredTotalXP)
		{
			Index = i;
		}
		else
		{
			break;
		}
	}

	Out.RankIndex = Index;
	Out.RankTitle = Rows[Index]->RankTitle;
	Out.CurrentRankFloorXP = Rows[Index]->RequiredTotalXP;

	if (Index + 1 < Rows.Num())
	{
		Out.NextRankXP = Rows[Index + 1]->RequiredTotalXP;
		Out.bIsMaxRank = false;
	}
	else
	{
		Out.NextRankXP = Out.CurrentRankFloorXP;
		Out.bIsMaxRank = true;
	}

	return Out;
}

FText UPGAdvancedFriendsGameInstance::GetRankTitleByIndex(int32 RankIndex)
{
	UDataTable* Table = GetRankTable();
	if (!Table)
	{
		return FText::GetEmpty();
	}

	TArray<FPGRankRow*> Rows;
	Table->GetAllRows<FPGRankRow>(TEXT("GetRankTitleByIndex"), Rows);
	Rows.Sort([](const FPGRankRow&A , const FPGRankRow& B)
	{
		return A.RequiredTotalXP < B.RequiredTotalXP;
	});

	if (Rows.IsValidIndex(RankIndex))
	{
		return Rows[RankIndex]->RankTitle;
	}

	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE