// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "Enemy/Common/AI/Interfaces/PGAIExplorationInterface.h"
#include "PGDeafCharacter.generated.h"

class UPGDeafAttributeSet;
/**
 * 
 */

UENUM(BlueprintType)
enum class EDeafHuntLevel : uint8
{
	Exploration,
	Chase,
	Scan,
	Count
};

UCLASS()
class PROJECTG_API APGDeafCharacter : public APGEnemyCharacterBase, public IPGAIExplorationInterface
{
	GENERATED_BODY()
	
public:

	APGDeafCharacter();
	
	// IPGAIExplorationInterface
	virtual float GetExplorationRadius() const override;
	virtual float GetExplorationWaitTime() const override;
	// ~IPGAIExplorationInterface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	EDeafHuntLevel GetHuntLevel() const;
	void SetHuntLevel(EDeafHuntLevel newHuntLevel);

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UPGDeafAttributeSet> DeafAttributeSet;

	virtual void BeginPlay() override;

	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationRadius = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	float ExplorationWaitTime = 3.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", Replicated, meta = (AllowPrivateAccess = "true"))
	EDeafHuntLevel huntLevel = EDeafHuntLevel::Exploration;
};
