// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "PGTriggerGimmickRockingChair.generated.h"

class USphereComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGTriggerGimmickRockingChair : public APGTriggerGimmickBase
{
	GENERATED_BODY()

public:
	APGTriggerGimmickRockingChair();

	// IPhotographableInterface~
	virtual bool IsPhotographable() const override { return bIsRocking; }
	virtual float GetPhotoDetectionRange() const override { return 2000.0f; }
	virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
	virtual FVector GetPhotoTargetLocation() const override;
	// ~IPhotographableInterface

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	UFUNCTION()
	void OnRep_IsRocking();

	void PlayCreakSound(bool FlipFlop);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(ReplicatedUsing = OnRep_IsRocking, VisibleAnywhere, BlueprintReadOnly, Category = "Chair")
	bool bIsRocking = false;

	/** 최대 기울기 각도 */
	UPROPERTY(EditAnywhere, Category = "Chair")
	float RockAngle = 8.0f;

	/** 왕복 1회에 걸리는 시간 */
	UPROPERTY(EditAnywhere, Category = "Chair")
	float RockPeriod = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	FName CreakSoundName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	FName Creak2SoundName;

	bool CreakSoundFlipFlopState = true;

	float RockElapsed = 0.0f;
	FRotator BaseRotation;

	/** 소리 재생 시점 추적용 */
	int32 LastCreakHalfCycle = -1;	
};
