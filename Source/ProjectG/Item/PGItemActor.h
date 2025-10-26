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

	void InitWithData(UPGItemData* InItemData);

	//IInteractableActorInterface~
	//Return Interact Ability of actor.
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	//~IInteractableActorInterface end

	//IItemInteractInterface~
	virtual UPGItemData* GetItemData() override;
	//~IItemInteractInterface end

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// When player drop item, this two functions are used to spawn item.
	void DropItemSpawned(const FRotator DropRotation);

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

	// replicate용 약한참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"), Replicatedusing = OnRep_ItemData)
	TSoftObjectPtr<UPGItemData> ItemDataPath;

	// ItemData를 메모리에 유지시키기 위해 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGItemData> LoadedItemData;

	UFUNCTION()
	void OnRep_ItemData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	bool bOwned = false;
};
