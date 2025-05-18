// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"


#include "PGBlindCharacter.generated.h"

class UPGBlindAttributeSet;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGBlindCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface
{
	GENERATED_BODY()

	
public:

	APGBlindCharacter();

	virtual float GetExplorationRadius() const override;
	virtual float GetExplorationWaitTime() const override;


	float GetNoiseLevelThreshold() const;

	int GetHuntLevel() const;
	void SetHuntLevel(int Level);



protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGBlindAttributeSet> BlindAttributeSet;
	
	//virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	int HuntLevel = 0;







	//detect Noise



private:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseLevelThreshold = 1000.f;


};
