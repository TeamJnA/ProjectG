// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGInteractableGimmickLever.generated.h"

class APGMirrorRoom;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGInteractableGimmickLever : public APGInteractableGimmickBase
{
	GENERATED_BODY()
	
public:
	APGInteractableGimmickLever();

	void SetMasterRoom(APGMirrorRoom* InRoom);

	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual FInteractionInfo GetInteractionInfo() const override;

	void ActivateLever();

private:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ActivatePhysics();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName FrameFallSound;

	UPROPERTY()
	TWeakObjectPtr<APGMirrorRoom> MasterRoom;

	UPROPERTY(Replicated)
	bool bIsActivated = false;
};
