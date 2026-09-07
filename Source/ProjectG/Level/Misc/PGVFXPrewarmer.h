// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PGVFXPrewarmer.generated.h"

class UMediaPlayer;
class UMediaSource;

UCLASS()
class PROJECTG_API APGVFXPrewarmer : public AActor
{
	GENERATED_BODY()

public:
	APGVFXPrewarmer();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void StartPrewarm();
	void FinishPrewarm();

	UPROPERTY(VisibleAnywhere, Category = "Prewarm")
	TObjectPtr<USceneComponent> Root;

	/** 카메라 위젯 미디어 */
	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	TObjectPtr<UMediaPlayer> MediaPlayerToPrewarm;

	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	TObjectPtr<UMediaSource> MediaSourceToPrewarm;

	/** 프리워밍을 유지할 프레임 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	int32 FramesToHold = 30;

	/** 완료 후 대기 프레임 수 */
	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	int32 FramesToFade = 50;

	int32 FrameCounter = 0;
	int32 FadeCounter = 0;

	bool bHideStarted = false;
	bool bPrewarmDone = false;
};
