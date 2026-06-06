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

	UFUNCTION(BlueprintCallable)
	void TriggerSoundLocally(FName InSoundName, FVector SoundLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TriggerSoundForAllPlayers(FName InSoundName, FVector SoundLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void TriggerSoundWithNoise(FName InSoundName, FVector SoundLocation, bool bIntensedSound = false);

	UFUNCTION(Client, Unreliable)
	void Client_ReportSelfNoise(uint8 SoundLevel);

	float GetCurrentActionNoiseLevel() const;

protected:
	// Called when the game starts
	// virtual void BeginPlay() override;

	UPROPERTY(Replicated)
	TObjectPtr<APGSoundManager> SoundManager;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float CurrentActionNoiseLevel = 0.0f;
	float ActionNoiseReportedTime = -10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Noise")
	float ActionNoiseDecayDuration = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Noise")
	float ActionNoiseHoldDuration = 0.15f;
};
