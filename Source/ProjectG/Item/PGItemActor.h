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

	void InitWithData(UPGItemData* InData);

	//Return Interact Ability of actor.
	TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;

	//IItemInteractInterface~
	virtual UPGItemData* GetItemData() override;
	//~IItemInteractInterface end

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_ItemData)
	TObjectPtr<UPGItemData> ItemData;

	UFUNCTION()
	void OnRep_ItemData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	bool bOwned = false;
};
