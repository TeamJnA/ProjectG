// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PGInventoryComponent.generated.h"

class UPGItemData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTG_API UPGInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPGInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<TObjectPtr<UPGItemData>> Items;

	UFUNCTION(BlueprintCallable)
	void ChangeCurrectItemIndex(int32 NewItemIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
	int32 CurrentItemIndex = 0;

	UFUNCTION()
	void AddItemToInventory(UPGItemData* ItemData);

	bool IsInventoryFull();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	int32 MaxInventorySize;

	int32 InventoryItemCount;

	bool bInventoryFull;
};
