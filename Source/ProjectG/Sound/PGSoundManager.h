// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/PGSoundPlayData.h"
#include "PGSoundManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSoundManager, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API APGSoundManager : public AActor
{
	GENERATED_BODY()

public:	
	APGSoundManager();

public:	
	// Play sound for a only client who called this function. 
	void 	PlaySoundForSelf(const FName& SoundName);

	// Play sound for all players.
	UFUNCTION(Server, Reliable)
	void PlaySoundForAllPlayers(const FName& SoundName, const FVector& SoundLocation);

	//  Play sound for all players and make noise. All players and enemy AI can hear sound.
	UFUNCTION(Server, Reliable)
	void 	PlaySoundWithNoise(const FName& SoundName, const FVector& SoundLocation, const bool bIntensedSound = false);

private:
	UFUNCTION(NetMulticast, Reliable)
	void PlaySoundMulticast(const FName& SoundName, const FVector& SoundLocation);

	UPROPERTY()
	TMap<FName, FPGSoundPlayData> SoundDataMap;

	UPROPERTY()
	TObjectPtr<USoundAttenuation> BaseSoundAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (AllowPrivateAccess = true))
	bool bDebugSoundRange;
};