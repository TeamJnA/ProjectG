// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGInteractableGimmickArmorStand.generated.h"

class UBoxComponent;
class UCameraShakeSourceComponent;

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

	void CollapseArmor(AActor* Investigator = nullptr);

	// IInteractableActorInterface~
	// virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	// virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	virtual void GimmickInteract(AActor* Investigator);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraShakeSourceComponent> CameraShakeSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName ArmorStandCollapseSound;

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText BreakText = FText::FromString(TEXT("Break"));

	UPROPERTY(ReplicatedUsing = OnRep_CollisionDisabled)
	bool bIsCollisionDisabled;

	UFUNCTION()
	void OnRep_CollisionDisabled();

	bool bAlreadyCollapsed;
};
