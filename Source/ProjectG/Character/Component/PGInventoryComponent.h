// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "PGInventoryComponent.generated.h"

class UPGItemData;
class APGPlayerCharacter;
class UAbilitySystemComponent;


//Store item data and ability spec handle from ability in item data.
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UPGItemData> ItemData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FGameplayAbilitySpecHandle ItemAbilitySpecHandle;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPGInventoryComponent();
	
protected:
	TObjectPtr<APGPlayerCharacter> PlayerCharacter;

	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	TArray<FInventoryItem> InventoryItems;

	//Changes the current inventory index and updates the active ability.
	UFUNCTION(BlueprintCallable)
	void ChangeCurrentInventoryIndex(int32 NewItemIndex);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 CurrentInventoryIndex = 0;

	UFUNCTION(Server, Reliable)
	void SetCurrentInventoryIndex(int32 NewIndex);

	//When GA_Interact(GA_Interact_Item) get itemData from ItemActor, this function is called to add item to inventory.
	UFUNCTION()
	void AddItemToInventory(UPGItemData* ItemData);

	// Activate current item's ability.
	void ActivateCurrentItemAbility();

	// Deactivate current item's ability.
	void DeactivateCurrentItemAbility();

	// Remove Item on current item index.
	void RemoveCurrentItem();

	void DropCurrentItem(const FVector& DropLocation);

	bool IsInventoryFull();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	int32 MaxInventorySize;

	//item counts in Inventory. If it's full, bInventoryFull become true.
	int32 InventoryItemCount;

	bool bInventoryFull;

	// Drop item to spawn
	UPROPERTY()
	TSubclassOf<class APGItemActor> ItemActor;
};
