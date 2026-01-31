// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"

#include "Engine/StreamableManager.h"

#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Game/PGGameState.h"

#include "PGAdvancedFriendsGameInstance.generated.h"

class UPGItemData;
class FUniqueNetId;
class FOnlineSessionSearchResult;
class UUserWidget;

static const FName SESSION_KEY_CURRENT_PLAYERS = FName(TEXT("CURRENT_PLAYERS"));

USTRUCT(BlueprintType)
struct FSteamFriendInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Friends")
	FString DisplayName;

	TSharedPtr<const FUniqueNetId> NetId;

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Friends")
	TObjectPtr<UTexture2D> Avatar;

	UPROPERTY(BlueprintReadOnly, Category = "Steam|Friends")
	bool bIsOnline = false;
};

DECLARE_DELEGATE_OneParam(FOnItemDataLoaded, UPGItemData*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionsFoundDelegate, const TArray<FOnlineSessionSearchResult>&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFriendListUpdatedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHostSessionAttemptStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHostSessionAttempFinishedDelegate, bool, bWasSuccesful, const FText&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFindSessionAttemptStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionAttemptFinishedDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinSessionAttemptStartedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJoinSessionAttemptFinishedDelegate, bool, bWasSuccesful, const FText&, ErrorMessage);

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAdvancedFriendsGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxInventorySize() const;

	bool IsHost() const { return bIsHost; }


	// --------- Session ---------
	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void HostSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate);

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void JoinFoundSession(int32 SessionIndex);

	void LeaveSessionAndReturnToMainMenu();

	// 게임 시작 후 GM에서 세션에 더이상 참가하지 못하도록 세팅
	void CloseSession();
	// 로비로 돌아온 경우 세션에 다시 참가할 수 있도록 세팅
	void OpenSession();

	FORCEINLINE const TArray<FUniqueNetIdRepl>& GetExpectedPlayersForTravel() const { return ExpectedPlayersForTravel; }
	void SetExpectedPlayersForTravel(const TArray<TObjectPtr<APlayerState>>& InPlayerArray);
	void ClearExpectedPlayersForTravel();

	void KickPlayerFromSession(const FUniqueNetId& PlayerToKickId);

	void UpdateSessionPlayerCount(int32 CurrentPlayers);

	FORCEINLINE void SetPendingNetworkFailureMessage(const FString& Message) { PendingNetworkFailureMessage = Message; }
	FORCEINLINE const FString& GetPendingNetworkFailureMessage() const { return PendingNetworkFailureMessage; }
	FORCEINLINE void ClearPendingNetworkFailureMessage() { PendingNetworkFailureMessage.Empty(); }

	FOnSessionsFoundDelegate OnSessionsFound;
	FOnHostSessionAttemptStartedDelegate OnHostSessionAttemptStarted;
	FOnHostSessionAttempFinishedDelegate OnHostSessionAttemptFinished;
	FOnFindSessionAttemptStartedDelegate OnFindSessionAttemptStarted;
	FOnFindSessionAttemptFinishedDelegate OnFindSessionAttemptFinished;
	FOnJoinSessionAttemptStartedDelegate OnJoinSessionAttemptStarted;
	FOnJoinSessionAttemptFinishedDelegate OnJoinSessionAttemptFinished;
	// --------- Session ---------


	// -------- Item --------
	UPGItemData* GetItemDataByKey(FName Key);

	void RequestLoadItemData(FName Key, FOnItemDataLoaded OnLoadedDelegate);
	// -------- Item --------


	// -------- Save current game state --------
	void SaveGameStateOnTravel(const EGameState StateToSave) { CurrentSavedGameState = StateToSave; }
	EGameState LoadGameStateOnTravel() const { return CurrentSavedGameState; }
	// -------- Save current game state --------


	// -------- Steam Friend --------
	void ReadSteamFriends();

	UTexture2D* GetSteamAvatarAsTexture(const FUniqueNetId& InUserId);
	
	// Invite
	void InviteFriend(const FUniqueNetId& FriendToInvite);

	FOnFriendListUpdatedDelegate OnFriendListUpdated;

	TArray<FSteamFriendInfo> CachedFriends;	
	// -------- Steam Friend --------


	// -------- LoadingScreen --------
	void ShowLoadingScreen();
	void HideLoadingScreen();
	// -------- LoadingScreen --------


protected:
	virtual void Init() override;

	// -------- Handle Failure --------
	// void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	// -------- Handle Failure --------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 MaxInventorySize = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSoftObjectPtr<UPGItemData>> ItemDataMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	EGameState CurrentSavedGameState;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

private:	
	// ------- Session --------
	// 세션 생성 완료 시 호출
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	// 세션 서칭 완료 시 호출
	void OnFindSessionsComplete(bool bWasSuccessful);
	// 세션 참가 완료 시 호출
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	// 세션 파괴 완료 시 호출
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	// 세션 초대 수락 시 호출
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);
	// 세션 세팅 업데이트 시 호출
	void OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful);

	void CreateNewSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate);
	void ForceReturnToMainMenu();

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> LatestSessionSearch;
	// 초대 정보 임시 저장
	TSharedPtr<FOnlineSessionSearchResult> AcceptedInviteInfo;

	// for debuging
	TSharedPtr<const FUniqueNetId> PendingHostId;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingScreenWidget;

	bool bIsHostingAfterDestroy;

	FString PendingNetworkFailureMessage;
	FName PendingSessionName;
	int32 PendingMaxPlayers;
	bool bIsPendingSessionPrivate;

	TArray<FUniqueNetIdRepl> ExpectedPlayersForTravel;
	// ------- Session --------

	bool bIsHost = false;

	FStreamableManager StreamableManager;

	// -------- Steam Friend -------------
	void OnReadFriendsListComplete(int32 LocalUserName, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	// -------- Steam Friend -------------

};
