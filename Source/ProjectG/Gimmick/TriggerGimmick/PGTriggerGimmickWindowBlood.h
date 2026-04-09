// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Interface/PhotographableInterface.h"
#include "PGTriggerGimmickWindowBlood.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API APGTriggerGimmickWindowBlood : public APGTriggerGimmickBase, public IPhotographableInterface
{
	GENERATED_BODY()
	
public:
	APGTriggerGimmickWindowBlood();

	// IPhotographableInterface~
	virtual bool IsPhotographable() const override;
	virtual float GetPhotoDetectionRange() const override { return 1500.0f; }
	virtual FPhotoSubjectInfo GetPhotoSubjectInfo() const override;
	virtual FVector GetPhotoTargetLocation() const override;
	// ~IPhotographableInterface

protected:
	virtual void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;
	virtual void LocalEffect(AActor* OtherActor, UPrimitiveComponent* OtherComp) override;

	void StartEffect();
	void SingleEffect();

	template<typename T>
	void ShuffleTArray(TArray<T>& Array);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TArray<TObjectPtr<UStaticMeshComponent>> EffectArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName WindowHitSoundName = FName("SFX_WindowHit");

	FTimerHandle EffectTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "0.0"))
	float MaxEffectTime = 0.24f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta = (ClampMin = "0.0"))
	float MinEffectTime = 0.08f;

	float TimeBetweenEffect = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	int32 MinEffect = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	int32 MaxEffect = 9;

	int32 NumEffect = 3;

	int32 CurrentIndex = 0;

	bool bIsActivated = false;
};
