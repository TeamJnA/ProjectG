// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"

#include "Engine/StreamableManager.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Game/PGGameState.h"

#include "PGAdvancedFriendsGameInstance.generated.h"

class UPGItemData;
class FUniqueNetId;

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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnJoinSessionBPCompleteDelegate, bool /*bWasSuccessful*/);
DECLARE_DELEGATE_OneParam(FOnItemDataLoaded, UPGItemData*);
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
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	int32 GetExpectedPlayerCount();
	void SetExpectedPlayerCount(int32 PlayerCount);

	void InitiateTravelTimer();
	void NotifyTravelSuccess();
	bool CheckIsTimerActive();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxInventorySize() const;

	bool IsHost() { return bIsHost; }

	//UFUNCTION(BlueprintCallable)
	//void HostSession(FName SessionName = FName("GameSession"), int32 MaxPlayers = 4);

	//UFUNCTION(BlueprintCallable)
	//void JoinSession(int32 SessionIndex);

	//UFUNCTION(BlueprintCallable)
	//void FindSessions();

	UPGItemData* GetItemDataByKey(FName Key);

	void RequestLoadItemData(FName Key, FOnItemDataLoaded OnLoadedDelegate);

	UPROPERTY(BlueprintReadOnly)
	FString Playername;

	FOnJoinSessionBPCompleteDelegate OnJoinSessionBPComplete;
	
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void HandleOnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "Networking|Session")
	void HandleOnJoinSessionComplete(bool bWasSuccessful);

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

	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FString& NewName);

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
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TSharedPtr<const FUniqueNetId> UserID;
	
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
