// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGAmbientSoundComponent.generated.h"

class UPGEnemyRegistry;
class APGPlayerCharacter;

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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Enemy 및 사망한 캐릭터 같은 것들 최초 발견 시 사운드 재생 용도.
	void OnAmbientSoundTrigger();

private:
	// 처음 볼 시 효과음을 재생할 Actor들을 등록하는 과정
	UFUNCTION()
	void InitTargetActors();

	// PGEnemyRegistry에 등록된 Enemy들을 추적 목록에 저장.
	void InitTrackedEnemies();

	// 로컬 캐릭터를 제외한 플레이어 캐릭터들을 추적 목록에 저장.
	void InitTrackedPlayerCharacters();

	// 주기적으로 거리와 화면에 보이는 상태를 갱신.
	void StartAwarenessUpdates();
	void UpdateAwareness();

	bool IsActorVisibleOnScreen(const AActor* TargetActor, APlayerController* PlayerController) const;
	bool IsPlayerDead(APGPlayerCharacter* PlayerCharacter) const;

	// 이미 확인한 에너미인지 확인하기 위한 Set
	TSet<TWeakObjectPtr<AActor>> SeenEnemies;

	TArray<TWeakObjectPtr<AActor>> TrackedEnemies;

	TArray<TWeakObjectPtr<APGPlayerCharacter>> TrackedPlayerCharacters;

	TMap<TWeakObjectPtr<AActor>, float> EnemyDistances;

	FTimerHandle AwarenessUpdateTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Sound")
	float AwarenessUpdateInterval = 0.3f;

	bool bEnterSequenceBinded = false;
	bool bFirstCorpseDiscovered = false;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
