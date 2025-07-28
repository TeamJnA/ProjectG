// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "Containers/List.h"



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

	// IPGAIExplorationInterface~
	// explore 기능 쓰려면 해당 interface 상속 받아야합니다.
	virtual float GetExplorationRadius() const override;
	virtual float GetExplorationWaitTime() const override;
	// ~IPGAIExplorationInterface


	float GetNoiseLevelThreshold() const;
	float GetNoiseMaxThreshold() const;

	int GetHuntLevel() const;
	void SetHuntLevel(int Level);



	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	//bite 용 collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UBoxComponent> BiteCollider;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UBoxComponent> DoorDetectCollider;

	//Animation Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BiteMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RoarMontage;

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

	// When BlindCharacter's HuntLevel is over than 0( State is Chase or Investigate ), 
	// the door ovelaped with BlindCharacter need to be broken.
	UFUNCTION()
	void OnOpenDoorColliderOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	void ForceOpenDoorsAroundCharacter();
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;


	//hunt level은 animation bp 추적용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", Replicated, meta = (AllowPrivateAccess = "true"))
	int HuntLevel = 0;

private:

	//아래는 detect Noise 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseLevelThreshold = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseMaxThreshold = 2000.f;


	//detect touch
private:

	//다수의 적 인식 개발중 (신경 x)
	//TLinkedList<AActor*> TouchedActorList;
	//TMap<AActor*, TLinkedList<AActor*>::> ActorNodeMap;

};
