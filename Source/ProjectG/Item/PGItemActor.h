// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Item/PGItemData.h"
#include "Interface/InteractableActorInterface.h"
#include "Interface/ItemInteractInterface.h"

#include "PGItemActor.generated.h"

UCLASS()
class PROJECTG_API APGItemActor : public AActor, public IInteractableActorInterface, public IItemInteractInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGItemActor();

	UFUNCTION(NetMulticast, Reliable)
	void InitWithData(UPGItemData* InData);

	//Return Interact Ability of actor.
	TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;

	//IItemInteractInterface~
	virtual UPGItemData* GetItemData() override;
	//~IItemInteractInterface end

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// When player drop item, this two functions are used to spawn item.
	void DropItemSpawned();

	UFUNCTION()
	void StopItemOnGroundHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGItemData> ItemData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	bool bOwned = false;


};
