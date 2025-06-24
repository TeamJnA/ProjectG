// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

public:	

	// Play sound for a only client who called this function. 
	void 	PlaySoundForSelf(USoundBase* SoundAsset, uint8 SoundVolumeLevel);

	// 
	// 	PlaySoundForAllPlayers()
	//  Server_PlaySoundForAllPlayers()
	// 

	UFUNCTION(Server, Reliable)
	void 	PlaySoundWithNoise(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundVolumeLevel, uint8 SoundRangeLevel);

	// 	Server_PlaySoundWithNoise()

	// HasAuthority�� �Լ� ���ο��� �˾Ƽ� �����ұ�.

private:
	//UFUNCTION(Client, Reliable)
	//void PlaySoundClient(USoundBase* SoundAsset, uint8 SoundVolumeLevel);

	UFUNCTION(NetMulticast, Reliable)
	void PlaySoundMulticast(USoundBase* SoundAsset, FVector SoundLocation, uint8 SoundVolumeLevel, uint8 SoundRangeLevel);
};

/*
Server	-> Server	0 �۵�	���� ȣ�� (���ø����̼� �ƴ�)
Client	-> Client	x �۵� ����	Ŭ���̾�Ʈ�� �ڱ� �ڽſ��� Ŭ���̾�Ʈ �Լ� ȣ�� �Ұ�
Client	-> NetMulticast	x �۵� ����	Ŭ���̾�Ʈ�� ��Ƽĳ��Ʈ ȣ�� ���� ����
Server	-> NetMulticast	0 �۵�	���� �� ��� Ŭ���̾�Ʈ �� �ڱ� �ڽ�
*/