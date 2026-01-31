// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_PGCharacterPlaySound.generated.h"

UENUM(BlueprintType)
enum class EPGANSoundType : uint8
{
	OwnerOnly      UMETA(DisplayName = "Owner Only"),
	AllPlayers     UMETA(DisplayName = "All Players"),
	PlayersAndAI   UMETA(DisplayName = "Players and AI (With Noise)")
};

/**
 * When with AllPlayers enum, it makes sound locally by replicated local anim.
 */
UCLASS()
class PROJECTG_API UAN_PGCharacterPlaySound : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_PGCharacterPlaySound();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName SoundName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	EPGANSoundType SoundType;
};
