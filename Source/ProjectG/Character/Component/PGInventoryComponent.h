// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpec.h"
#include "PGInventoryComponent.generated.h"


class APGItemActor;
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


DECLARE_DYNAMIC_DELEGATE(FOnInventoryComponentReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentSlotIndexChanged, int32, NewIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemUpdate, const TArray<FInventoryItem>&, InventoryItems);


DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPGInventoryComponent();
	
protected:
	virtual void BeginPlay() override;

public:	
	//Changes the current inventory index and updates the active ability.
	UFUNCTION(BlueprintCallable)
	void ChangeCurrentInventoryIndex(const int32 NewItemIndex);

	UFUNCTION(Server, Reliable)
	void SetCurrentInventoryIndex(const int32 NewIndex);

	// When Interact Ability(GA_Interact_Item) get itemData from ItemActor, this function is called.
	UFUNCTION()
	void AddItemToInventory(UPGItemData* ItemData);

	void ActivateCurrentItemAbility();

	void DeactivateCurrentItemAbility();

	void RemoveCurrentItem();

	void DropCurrentItem(const FVector DropLocation);

	bool IsInventoryFull() { return	bInventoryFull; };

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_InventoryItems, EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TArray<FInventoryItem> InventoryItems;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	int32 CurrentInventoryIndex;

	TObjectPtr<APGPlayerCharacter> PlayerCharacter;

	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	int32 MaxInventorySize;

	// Item counts in Inventory. If it's full, bInventoryFull become true.
	int32 InventoryItemCount;

	bool bInventoryFull;

	// Drop item to spawn
	UPROPERTY()
	TSubclassOf<APGItemActor> ItemActor;

protected:
	UFUNCTION()
	void OnRep_InventoryItems();


/*
	UI part
*/
public:
	UPROPERTY()
	FOnInventoryComponentReady OnInventoryComponentReady;

	UPROPERTY()
	FOnInventoryItemUpdate OnInventoryItemUpdate;

	FOnCurrentSlotIndexChanged OnCurrentSlotIndexChanged;
};
