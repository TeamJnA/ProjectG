// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PGAdvancedFriendsGameInstance.generated.h"

class UPGItemData;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGAdvancedFriendsGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	void MarkClientTravelled();
	void ResetClientTravelFlag();
	bool HasClientTravelled() const;

	void IncrementTravelRetryCount();
	void ResetTravelRetryCount();
	bool HasExceededRetryLimit() const;
	void LeaveSessionAndReturnToLobby();
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	int32 GetExpectedPlayerCount();
	void SetExpectedPlayerCount(int32 PlayerCount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxInventorySize() const;

	//UFUNCTION(BlueprintCallable)
	//void HostSession(FName SessionName = FName("GameSession"), int32 MaxPlayers = 4);

	//UFUNCTION(BlueprintCallable)
	//void JoinSession(int32 SessionIndex);

	//UFUNCTION(BlueprintCallable)
	//void FindSessions();

	UPGItemData* GetItemDataByKey(FName Key);

	UPROPERTY(BlueprintReadOnly)
	FString Playername;

protected:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FString& NewName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 MaxInventorySize = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSoftObjectPtr<UPGItemData>> ItemDataMap;

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TSharedPtr<const FUniqueNetId> UserID;
	bool bDidClientTravel = false;
	
	int32 ExpectedPlayerCount = 0;
	int32 TravelRetryCount = 0;

	//void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	//void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	//void OnFindSessionsComplete(bool bWasSuccessful);
};
