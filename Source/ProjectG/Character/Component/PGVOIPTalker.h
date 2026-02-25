// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/VoiceConfig.h"
#include "PGVOIPTalker.generated.h"

class APGPlayerCharacter;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGVOIPTalker : public UVOIPTalker
{
	GENERATED_BODY()

public:
    static UPGVOIPTalker* CreateTalkerForPlayer(APlayerState* OwningState);

    /**
     * Override from UVOIPTalker - called when a player starts talking.
     * This is where we capture the AudioComponent and apply volume settings.
     */
    virtual void OnTalkingBegin(UAudioComponent* AudioComponent) override;

    /**
     * Override from UVOIPTalker - called when a player stops talking.
     */
    virtual void OnTalkingEnd() override;

    void UpdateVolume();

private:
    /**
     * Weak reference to the owner character.
     * We use a weak reference to avoid circular references.
     */
    UPROPERTY()
    TObjectPtr<APlayerState> CachedPlayerState;

    UPROPERTY()
    TWeakObjectPtr<UAudioComponent> CachedAudioComponent;

    /**
     * Get the player volume multiplier from the VoiceVolumeManager.
     * Returns 1.0f if no manager is found or player is not found.
     */
    float GetPlayerVolumeMultiplier() const;
};
