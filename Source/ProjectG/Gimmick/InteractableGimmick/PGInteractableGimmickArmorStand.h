// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGInteractableGimmickArmorStand.generated.h"

class UBoxComponent;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGInteractableGimmickArmorStand : public APGInteractableGimmickBase
{
	GENERATED_BODY()
	
public:
	APGInteractableGimmickArmorStand();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void CollapseArmor();

	// IInteractableActorInterface~
	// virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	// virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	virtual void GimmickInteract();

protected:
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ArmorMesh", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UStaticMeshComponent>> ArmorMeshs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ArmorBoxCollision;

	UPROPERTY(ReplicatedUsing = OnRep_CollisionDisabled)
	bool bIsCollisionDisabled;

	UFUNCTION()
	void OnRep_CollisionDisabled();
};
