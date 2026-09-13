// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGAmbientSoundComponent.generated.h"

class UPGEnemyRegistry;
class APGPlayerCharacter;
class UAudioComponent;
class USoundBase;

/*
* 기존에는 일반적인 Ambient Sound를 Meta Sound 기반으로 재생.
* 그러다 2가지 경우 Tension Sound 재생으로 변경
* 1. 몬스터를 처음 본 경우, 시체를 처음 본 경우
* 2. 몬스터가 주변에 존재해서 긴장감이 오름
* 2-1. 긴장감이 일정 수치 이상일 때, 주변 Gimmick들로부터 놀람이 전해짐
* 2-2. 긴장감이 일정 수치를 넘었을 때.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGAmbientSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPGAmbientSoundComponent();

	/*
	* 입장 컷씬이 끝나는 것을 구독.Begin play에서 시도 및 Player Character에서 Controller 도착 시 시도.
	* Locally Controlled를 체크해야 하는데, Beginplay 시에는 아직 갱신이 안되었을 수도 있다.
	* 컷씬이 끝난 후, 함수 진행으로 각종 액터들의 위치를 추적.
	*/
	void TryBindEnterSequenceFinishDelegate();

	// 주변에서 기믹이 작동했을 경우, 긴장감 수치를 확 올리고 소리 재생 확인.
	void GimmickTriggerTension();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Enemy 및 사망한 캐릭터 같은 것들 최초 발견 시 사운드 재생 용도.
	void OnAmbientSoundTrigger();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visible", meta = (AllowPrivateAccess = "true"))
	float MaxVisibilityDistance = 3000.0f;

private:
	// 처음 볼 시 효과음을 재생할 Actor들을 등록하는 과정
	UFUNCTION(BlueprintCallable)
	void InitTargetActors();

	// PGEnemyRegistry에 등록된 Enemy들을 추적 목록에 저장.
	void InitTrackedEnemies();

	// 로컬 캐릭터를 제외한 플레이어 캐릭터들을 추적 목록에 저장.
	void InitTrackedPlayerCharacters();

	void InitAmbientAudioComponent();

	// 주기적으로 거리와 화면에 보이는 상태를 갱신.
	void StartAwarenessUpdates();
	void UpdateAwareness();

	bool IsActorVisibleOnScreen(const AActor* TargetActor, APlayerController* PlayerController) const;
	bool IsPlayerDead(APGPlayerCharacter* PlayerCharacter) const;

	void ResetTensionReady();

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> AmbientMetaSound;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> AmbientAudioComponent;

	// 이미 확인한 에너미인지 확인하기 위한 Set
	TSet<TWeakObjectPtr<AActor>> SeenEnemies;

	TArray<TWeakObjectPtr<AActor>> TrackedEnemies;

	TArray<TWeakObjectPtr<APGPlayerCharacter>> TrackedPlayerCharacters;

	FTimerHandle AwarenessUpdateTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Sound")
	float AwarenessUpdateInterval = 0.3f;

	bool bEnterSequenceBinded = false;
	bool bFirstCorpseDiscovered = false;

	// Tension per second =  Time * AwarenessUpdateInterval
	UPROPERTY(EditDefaultsOnly, Category = "Tension")
	float GimmickTensionThreshold = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Tension")
	float BaseTensionThreshold = 100;

	float CurTensionValue = 0.0f;

	FTimerHandle TensionCooldownTimerHandle;
	float TensionRepeatDelayTime = 25.0f;
	bool bIsTensionReady = true;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
