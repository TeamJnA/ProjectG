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

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAdvancedFriendsGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	bool DidRetryClientTravel() const;
	void LeaveSessionAndReturnToLobby();

	int32 GetExpectedPlayerCount();
	void SetExpectedPlayerCount(int32 PlayerCount);

	void InitiateTravelTimer();
	void NotifyTravelSuccess();
	bool CheckIsTimerActive();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxInventorySize() const;

	bool IsHost() { return bIsHost; }

	// --------- Session ---------
	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void HostSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate);

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void JoinFoundSession(int32 SessionIndex);

	FOnSessionsFoundDelegate OnSessionsFound;
	// --------- Session ---------

	UPGItemData* GetItemDataByKey(FName Key);

	void RequestLoadItemData(FName Key, FOnItemDataLoaded OnLoadedDelegate);

	UPROPERTY(BlueprintReadOnly)
	FString Playername;

	// GameState
	void SaveGameStateOnTravel(EGameState StateToSave);
	EGameState LoadGameStateOnTravel();

	// -------- Steam Friend -------------
	void ReadSteamFriends();

	bool GetSteamAvatarAsRawData(const FUniqueNetId& InUserId, TArray<uint8>& OutRawData, int32& OutWidth, int32& OutHeight);
	UTexture2D* GetSteamAvatarAsTexture(const FUniqueNetId& InUserId);
	
	// Invite
	void InviteFriend(const FUniqueNetId& FriendToInvite);

	FOnFriendListUpdatedDelegate OnFriendListUpdated;

	TArray<FSteamFriendInfo> CachedFriends;
	
	// -------- Steam Friend -------------

protected:
	virtual void Init() override;

	// void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
	void OnTravelTimeout();

	void NotifyTravelFailed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 MaxInventorySize = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSoftObjectPtr<UPGItemData>> ItemDataMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	EGameState CurrentSavedGameState;

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

	void CreateNewSession(FName SessionName, int32 MaxPlayers, bool bIsPrivate);

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> LatestSessionSearch;
	//TSharedPtr<FOnlineSessionSettings> LatestSessionSettings;
	// 초대 정보 임시 저장
	TSharedPtr<FOnlineSessionSearchResult> AcceptedInviteInfo;

	bool bIsHostingAfterDestroy;

	FName PendingSessionName;
	int32 PendingMaxPlayers;
	bool bIsPendingSessionPrivate;
	// ------- Session --------

	int32 ExpectedPlayerCount = 0;
	int32 TravelRetryCount = 0;
	FTimerHandle TravelTimerHandle;
	bool bDidRetryClientTravel = false;
	bool bTimeoutProcessInProgress = false;
	bool bOnTravelFailureDetected = false;

	bool bIsHost = false;

	FStreamableManager StreamableManager;

	// -------- Steam Friend -------------
	void OnReadFriendsListComplete(int32 LocalUserName, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
	// -------- Steam Friend -------------

};
