// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Game/PGGameState.h"

#include "Player/PGLobbyPlayerController.h"
#include "Player/PGPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Engine/Texture2D.h"

#include "Item/PGItemData.h"
#include "Engine/StreamableManager.h"

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

bool UPGAdvancedFriendsGameInstance::DidRetryClientTravel() const
{
	return bDidRetryClientTravel;
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

	IOnlineSessionPtr SessionInterfaceRef = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterfaceRef.IsValid())
	{
		// ���� �������̽��� �������� ������ ��� �κ�� �̵�
		UE_LOG(LogTemp, Warning, TEXT("GI::LeaveSessionAndReturnToLobby: Online Session Interface not valid. Forcing return to main menu."));
		UGameplayStatics::OpenLevel(this, FName("/Game/ProjectG/Levels/LV_PGLobbyRoom"), true);
		return;
	}

	// ���� �ı� �� ȣ��� ��������Ʈ ���ε�
	// ���� �ڵ��� �ִٸ� ����(�ߺ�����)		
	if (DestroySessionCompleteDelegateHandle.IsValid())
	{
		SessionInterfaceRef->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	DestroySessionCompleteDelegateHandle = SessionInterfaceRef->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UPGAdvancedFriendsGameInstance::OnDestroySessionComplete)
	);

	// ȣ��Ʈ�� Ŭ���̾�Ʈ�� ���� ���ǿ� �ִٸ� DestroySession�� ȣ��
	// ȣ��Ʈ -> ���� ��ü�� �ı�
	// Ŭ���̾�Ʈ -> �ش� ���ǿ��� '����' ó��
	const FName CurrentSessionName = NAME_GameSession; // �Ǵ� ���� ���� �� ����� �̸�
	if (!SessionInterfaceRef->DestroySession(CurrentSessionName))
	{
		// DestroySession ȣ�� ��ü�� ������ ��� (�ſ� �幮 ���) ��������Ʈ�� �ٷ� �����ϰ� �κ�� �̵�
		UE_LOG(LogTemp, Warning, TEXT("Failed to initiate session destruction for %s. Forcing return."), *CurrentSessionName.ToString());
		SessionInterfaceRef->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		// �� ��쿡�� OnDestroySessionComplete�� �������� ȣ���Ͽ� ���¸� �����ϰ� �̵��ϴ� ���� �� ����
		OnDestroySessionComplete(CurrentSessionName, false);
	}
	// ���������� DestroySession ȣ���� ���۵Ǿ��ٸ�, OnDestroySessionComplete �ݹ��� �� ������ ���
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
		UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: Session %s destroyed successfully"), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GI::OnDestroySessionComplete: Failed to destroy session %s"), *SessionName.ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("GI::OnDestroySessionComplete: Session destroy complete, back to lobby"));

	bDidRetryClientTravel = false;
	bTimeoutProcessInProgress = false;
	bOnTravelFailureDetected = false;
	TravelRetryCount = 0;

	CurrentSavedGameState = EGameState::MainMenu;
	bIsHost = false;

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
