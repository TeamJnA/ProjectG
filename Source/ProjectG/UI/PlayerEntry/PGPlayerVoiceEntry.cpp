// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerEntry/PGPlayerVoiceEntry.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/Image.h"
#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Character/Component/PGVOIPTalker.h"

void UPGPlayerVoiceEntry::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (VolumeSlider)
    {
        VolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UPGPlayerVoiceEntry::OnVolumeSliderChanged);
    }
}

void UPGPlayerVoiceEntry::SetupEntry(APlayerState* InPlayerState)
{
    if (!InPlayerState)
    {
        return;
    }

    bIsLoadingSettings = true;
    PlayerStateRef = InPlayerState;

    if (PlayerAvatar)
    {
        if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
        {
            if (InPlayerState->GetUniqueId().IsValid())
            {
                UTexture2D* AvatarTexture = GI->GetSteamAvatarAsTexture(*InPlayerState->GetUniqueId().GetUniqueNetId());
                if (AvatarTexture)
                {
                    PlayerAvatar->SetBrushFromTexture(AvatarTexture);
                }
                else
                {
                    PlayerAvatar->SetVisibility(ESlateVisibility::Hidden);
                }
            }
        }
    }

    if (PlayerNameText)
    {
        PlayerNameText->SetText(FText::FromString(InPlayerState->GetPlayerName()));
    }

    if (VolumeSlider)
    {
        if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
        {
            const float SavedVolume = GI->GetRemotePlayerVolume(InPlayerState->GetUniqueId());
            VolumeSlider->SetValue(SavedVolume);
        }
        else
        {
            VolumeSlider->SetValue(1.0f);
        }
    }

    bIsLoadingSettings = false;

    UE_LOG(LogTemp, Log, TEXT("[VoiceEntry] Setup entry for player: %s"), *InPlayerState->GetPlayerName());

}

void UPGPlayerVoiceEntry::OnVolumeSliderChanged(float NewValue)
{
    if (bIsLoadingSettings)
    {
        return;
    }

    APlayerState* PS = PlayerStateRef.Get();
    if (!PS)
    {
        return;
    }

    if (UPGAdvancedFriendsGameInstance* GI = GetGameInstance<UPGAdvancedFriendsGameInstance>())
    {
        GI->SetRemotePlayerVolume(PS->GetUniqueId(), NewValue);

        UE_LOG(LogTemp, Log, TEXT("[VoiceEntry] Set volume for %s to %.2f"), *PlayerStateRef->GetPlayerName(), NewValue);
    }

    if (UPGVOIPTalker* Talker = Cast<UPGVOIPTalker>(UVOIPStatics::GetVOIPTalkerForPlayer(PS->GetUniqueId())))
    {
        Talker->UpdateVolume();

        UE_LOG(LogTemp, Log, TEXT("[VoiceEntry] Set volume for %s to %.2f Immadiately"), *PlayerStateRef->GetPlayerName(), NewValue);
    }
}
