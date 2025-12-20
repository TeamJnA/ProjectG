
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGTriggerGimmickBase.generated.h"

class UBoxComponent;
class UAbilitySystemComponent;
class UGameplayEffect;
class APGSoundManager;

UCLASS(Abstract)
class PROJECTG_API APGTriggerGimmickBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGTriggerGimmickBase();

	void InitSoundManager();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	virtual void LocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp) { }
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp);

	TSet<TWeakObjectPtr<APlayerState>> TriggeredPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick")
	TSubclassOf<UGameplayEffect> TriggerEffectClass;

	UPROPERTY(Replicated)
	TObjectPtr<APGSoundManager> SoundManager;

	// 이벤트 발동 확률
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick")
	float ActivationChance = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick")
	bool bIsOneShotEvent = true;

};
