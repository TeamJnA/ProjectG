// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "Interface/PhotographableInterface.h"
#include "PGBlindCharacter.generated.h"

class UPGBlindAttributeSet;
class UBoxComponent;
class UAudioComponent;
class UCameraShakeSourceComponent;

/**
 * 시각이 없는 대신 소리와 촉각을 이용해 플레이어를 추적하는 적 캐릭터 클래스입니다.
 * 탐색(Exploration), 조사(Investigation), 추격(Chase)의 3단계 사냥 레벨을 가집니다.
 */

UENUM(BlueprintType)
enum class EBlindHuntLevel : uint8
{
	Exploration,
	Investigation,
	Chase,
	Count
}; 

UENUM(BlueprintType)
enum class EBlindSoundState : uint8
{
	Breathing,   // Exploration, Investigation
	Chasing,     // Chase 이동 중
	Growling,    // Chase 도착 후 대기
	Attacking,   // Bite
	Silent		 // Roar 중
};

UCLASS()
class PROJECTG_API APGBlindCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface, public IPhotographableInterface
{
	GENERATED_BODY()

public:
	APGBlindCharacter();

	// IPGAIExplorationInterface~
	FORCEINLINE virtual float GetExplorationRadius() const override { return ExplorationRadius; }
	FORCEINLINE virtual float GetExplorationWaitTime() const override { return ExplorationWaitTime; }
	// ~IPGAIExplorationInterface

	// IPhotographableInterface~
	virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
	virtual FVector GetPhotoTargetLocation() const override;
	// ~IPhotographableInterface

	FORCEINLINE int32 GetNoiseLevelThreshold() const { return NoiseLevelThreshold; }
	FORCEINLINE int32 GetNoiseMaxThreshold() const { return NoiseMaxThreshold; }

	FORCEINLINE EBlindHuntLevel GetHuntLevel() const { return HuntLevel; }
	void SetHuntLevel(EBlindHuntLevel newHuntLevel);
	void SetSoundState(EBlindSoundState NewState);
	FORCEINLINE EBlindSoundState GetSoundState() const { return SoundState; }
	void TriggerFootstepShake();
	void SpawnFootprint(const FVector& Location);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	FVector2D BreathInterval = FVector2D(1.5f, 3.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	FVector2D ChaseInterval = FVector2D(4.5f, 5.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	FVector FootprintDecalSize = FVector(10.0f, 20.0f, 20.0f);

public:
	//Animation Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BiteMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RoarMontage;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGBlindAttributeSet> BlindAttributeSet;

	virtual void BeginPlay() override;
	virtual void OnPlayerOverlapped(AActor* OverlapPlayer) override;

	//virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> BreathAudioComponent;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> ChaseAudioComponent;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> GrowlAudioComponent;

	UPROPERTY(VisibleAnywhere, Category = "CameraShake")
	TObjectPtr<UCameraShakeSourceComponent> CameraShakeSource;

	UPROPERTY(EditDefaultsOnly, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> WalkShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> RunShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	TObjectPtr<UMaterialInterface> FootprintDecalMaterial;

private:
	FTimerHandle SoundLoopTimerHandle;
	FTimerHandle GrowlDelayTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	float FootprintLifetime = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Footprint")
	float FootprintFadeDuration = 2.0f;

	//아래는 detect Noise 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	int32 NoiseLevelThreshold = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	int32 NoiseMaxThreshold = 3;

	//hunt level은 animation bp 추적용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", Replicated, meta = (AllowPrivateAccess = "true"))
	EBlindHuntLevel HuntLevel = EBlindHuntLevel::Exploration;

	UPROPERTY(ReplicatedUsing = OnRep_SoundState)
	EBlindSoundState SoundState = EBlindSoundState::Breathing;

	UFUNCTION()
	void OnRep_SoundState();

	void PlayBreathOnce();
	void PlayChaseOnce();
	void PlayGrowlOnce();
	void StopGrowl();
	void ScheduleNextSound();
};
