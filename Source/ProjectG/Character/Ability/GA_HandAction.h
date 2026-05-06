// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_HandAction.generated.h"

#define PG_CHECK_VALID_HANDACTION(Ptr) \
    if (!(Ptr)) \
    { \
        UE_LOG(LogTemp, Warning, TEXT(#Ptr " is null.")); \
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false); \
        return; \
    }

/**
 * 
 */
UCLASS()
class PROJECTG_API UGA_HandAction : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_HandAction();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnCompletedAnimMontage();

	/**
	* Hand Action anim montages
	*  Pick	*  Change	* Drop * CameraOn * CameraOff
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> HandActionAnimMontages;

	bool bCameraOnEnded;

	bool bAbilityEnded;

	// CameraOn이 Event로 들어올 경우, 어빌리티가 끝날 때 HandLock을 해줘야 함.
	bool bIsCameraOn;
};
