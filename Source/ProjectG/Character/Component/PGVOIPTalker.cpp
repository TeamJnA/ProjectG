// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGVOIPTalker.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"


float UPGVOIPTalker::GetPlayerVolumeMultiplier() const
{
    const float DefaultVolume = 1.0f;
    if (!CachedPlayerState)
    {
        return DefaultVolume;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PGVOIPTalker] GetWorld() returned nullptr"));
        return DefaultVolume;
    }

    UPGAdvancedFriendsGameInstance* GI = Cast<UPGAdvancedFriendsGameInstance>(World->GetGameInstance());
    if (!GI)
    {
        return DefaultVolume;
    }

    const FUniqueNetIdRepl& CachedPlayerId = CachedPlayerState->GetUniqueId();
    if (!CachedPlayerId.IsValid())
    {
        return DefaultVolume;
    }

    return GI->GetRemotePlayerVolume(CachedPlayerId);
}

UPGVOIPTalker* UPGVOIPTalker::CreateTalkerForPlayer(APlayerState* OwningState)
{
    if (!OwningState)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PGVOIPTalker] CreateTalkerForPlayer called with null OwningState"));
        return nullptr;
    }

    UPGVOIPTalker* NewTalker = NewObject<UPGVOIPTalker>(OwningState);
    if (NewTalker != nullptr)
    {
        NewTalker->CachedPlayerState = OwningState;
        NewTalker->RegisterWithPlayerState(OwningState);

        UE_LOG(LogTemp, Log, TEXT("[PGVOIPTalker] Created talker for player %s"),
            *OwningState->GetPlayerName());
    }

    return NewTalker;
}

void UPGVOIPTalker::OnTalkingBegin(UAudioComponent* AudioComponent)
{
    Super::OnTalkingBegin(AudioComponent);

    if (!AudioComponent)
    {
        return;
    }

    CachedAudioComponent = AudioComponent;

    const float PlayerVolume = GetPlayerVolumeMultiplier();
    AudioComponent->SetVolumeMultiplier(PlayerVolume);

    if (CachedPlayerState)
    {
        UE_LOG(LogTemp, Log, TEXT("[PGVOIPTalker] Applied volume (%.2f) to %s"),
            PlayerVolume, *CachedPlayerState->GetPlayerName());
    }
}

void UPGVOIPTalker::OnTalkingEnd()
{
    Super::OnTalkingEnd();
    CachedAudioComponent = nullptr;

    UE_LOG(LogTemp, Log, TEXT("[PGVOIPTalker] Player stopped talking"));
}

void UPGVOIPTalker::UpdateVolume()
{
    if (UAudioComponent* AC = CachedAudioComponent.Get())
    {
        const float PlayerVolume = GetPlayerVolumeMultiplier();
        AC->SetVolumeMultiplier(PlayerVolume);

        if (CachedPlayerState)
        {
            UE_LOG(LogTemp, Log, TEXT("[PGVOIPTalker] Applied volume (%.2f) to %s"), PlayerVolume, *CachedPlayerState->GetPlayerName());
        }
    }
}