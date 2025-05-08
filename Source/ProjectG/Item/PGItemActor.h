// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Item/PGItemData.h"
#include "GameplayTagAssetInterface.h"

#include "PGItemActor.generated.h"

UCLASS()
class PROJECTG_API APGItemActor : public AActor, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGItemActor();

	//IGameplayTagAssetInterface~
	//~IGameplayTagAssetInterface
	void InitWithData(UPGItemData* InData);
	//Send the Itemtag of itemdata by AddTag
	void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_ItemData)
	TObjectPtr<UPGItemData> ItemData;

	UFUNCTION()
	void OnRep_ItemData();

};
