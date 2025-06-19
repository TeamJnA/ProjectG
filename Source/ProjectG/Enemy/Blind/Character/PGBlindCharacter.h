// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"


#include "PGBlindCharacter.generated.h"

class UPGBlindAttributeSet;
class UBoxComponent;
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
	float GetNoiseMaxThreshold() const;

	int GetHuntLevel() const;
	void SetHuntLevel(int Level);



	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	//bite ¿ë collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	UBoxComponent* BiteCollider;



	//Animation Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BiteMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* RoarMontage;


protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGBlindAttributeSet> BlindAttributeSet;

	virtual void BeginPlay() override;
	
	//virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


	UFUNCTION()
	void OnBiteColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);





	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", Replicated, meta = (AllowPrivateAccess = "true"))
	int HuntLevel = 0;







	//detect Noise



private:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseLevelThreshold = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseMaxThreshold = 2000.f;


};
