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

	//IGameplayTagAssetInterface~
	//Return Interact Ability of actor.
	TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	//~IGameplayTagAssetInterface end

	//IItemInteractInterface~
	virtual UPGItemData* GetItemData() override;
	//~IItemInteractInterface end

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGItemData> ItemData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	bool bOwned = false;
};
