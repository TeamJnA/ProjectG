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
	//Send the Itemtag of itemdata by AddTag
	void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~IGameplayTagAssetInterface

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMesh;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemData", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGItemData> ItemData;
};
