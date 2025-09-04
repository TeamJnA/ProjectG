// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGSoundManagerComponent.generated.h"

class APGSoundManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGSoundManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPGSoundManagerComponent();

	void SetSoundManager(APGSoundManager* InSoundManager);

	UFUNCTION(BlueprintCallable)
	void TriggerSoundForSelf(FName InSoundName);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TriggerSoundForAllPlayers(FName InSoundName, FVector SoundLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TriggerSoundWithNoise(FName InSoundName, FVector SoundLocation, bool bIntensedSound = false);

protected:
	// Called when the game starts
	// virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	TObjectPtr<APGSoundManager> SoundManager;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
