// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/PGSoundPlayData.h"
#include "PGSoundManager.generated.h"

/*
	In future
	strurct EventParams{
	name,
	location,
	level,
	level
	}
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API APGSoundManager : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	APGSoundManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	TMap<FName, FPGSoundPlayData> SoundDataMap;

public:	

	// Play sound for a only client who called this function. 
	void 	PlaySoundForSelf(FName SoundName, uint8 SoundVolumeLevel);

	// Play sound for all players.
	UFUNCTION(Server, Reliable)
	void PlaySoundForAllPlayers(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel);

	//  Play sound for all players and make noise. All players and enemy AI can hear sound.
	UFUNCTION(Server, Reliable)
	void 	PlaySoundWithNoise(FName SoundName, FVector SoundLocation, uint8 SoundPowerLevel, bool bIntensedSound = false);

private:
	//UFUNCTION(Client, Reliable)
	//void PlaySoundClient(USoundBase* SoundAsset, uint8 SoundVolumeLevel);

	UFUNCTION(NetMulticast, Reliable)
	void PlaySoundMulticast(USoundBase* SoundAsset, float SoundStartTime, FVector SoundLocation, uint8 SoundVolumeLevel);

	UPROPERTY()
	TObjectPtr<USoundAttenuation> BaseSoundAttenuation;

	bool bDebugSoundRange;
};

/*
Server	-> [Server] 0 작동	로컬 호출 (리플리케이션 아님)
Client	-> [Client]	x 작동 안함	클라이언트는 자기 자신에게 클라이언트 함수 호출 불가
Client	-> [NetMulticast]	x 작동 안함	클라이언트는 멀티캐스트 호출 권한 없음
Server	-> [NetMulticast]	0 작동	서버 → 모든 클라이언트 및 자기 자신
*/