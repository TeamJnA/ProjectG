// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Item/PGItemData.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "PGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetMaxInventorySize() const;

	UFUNCTION(BlueprintCallable)
	void JoinSession(int32 SessionIndex);

	UFUNCTION(BlueprintCallable)
	void FindSessions();

	UPGItemData* GetItemDataByKey(FName Key);

	UPROPERTY(BlueprintReadOnly)
	FString Playername;

protected:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void HostSession(FName SessionName = FName("GameSession"), int32 MaxPlayers = 4);

	UFUNCTION(BlueprintCallable)
	void SetPlayerName(const FString& NewName);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 MaxInventorySize = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSoftObjectPtr<UPGItemData>> ItemDataMap;

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
