// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "PGBlindCharacter.generated.h"

class UPGBlindAttributeSet;

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

UCLASS()
class PROJECTG_API APGBlindCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface
{
	GENERATED_BODY()

public:

	APGBlindCharacter();

	// IPGAIExplorationInterface~
	virtual float GetExplorationRadius() const override { return ExplorationRadius; }
	virtual float GetExplorationWaitTime() const override { return ExplorationWaitTime; }
	// ~IPGAIExplorationInterface


	float GetNoiseLevelThreshold() const { return NoiseLevelThreshold; }
	float GetNoiseMaxThreshold() const { return NoiseMaxThreshold; }

	EBlindHuntLevel GetHuntLevel() const { return HuntLevel; }
	void SetHuntLevel(EBlindHuntLevel newHuntLevel);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Animation Montages
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> BiteMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> RoarMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> SniffMontage;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGBlindAttributeSet> BlindAttributeSet;

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;


	//hunt level은 animation bp 추적용 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", Replicated, meta = (AllowPrivateAccess = "true"))
	EBlindHuntLevel HuntLevel = EBlindHuntLevel::Exploration;

private:

	//아래는 detect Noise 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseLevelThreshold = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float NoiseMaxThreshold = 2000.f;
};
