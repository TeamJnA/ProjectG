// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGInventoryComponent.h"
#include "Item/PGItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Game/PGGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Character/PGPlayerCharacter.h"
#include "Type/CharacterTypes.h"
#include "AbilitySystemComponent.h"
#include "Item/PGItemActor.h"

// Sets default values for this component's properties
UPGInventoryComponent::UPGInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bInventoryFull = false;

	InventoryItemCount = 0;

	CurrentInventoryIndex = 0;

	SetIsReplicatedByDefault(true);

	static ConstructorHelpers::FClassFinder<APGItemActor> ItemActorRef(TEXT("/Game/ProjectG/Items/BP_PGItemActor.BP_PGItemActor_C"));
	if (ItemActorRef.Class)
	{
		ItemActor = ItemActorRef.Class;
	}
}


// Called when the game starts
void UPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get max Inventory size, set ItemInventory and ItemAbilitySpecHandle array to max size
	// If failed to get MaxInventorySize from GameInstance, Set max size to 5.
	UPGGameInstance* PGGameInstance = Cast<UPGGameInstance>
		(UGameplayStatics::GetGameInstance(GetWorld()));

	if (PGGameInstance) {
		MaxInventorySize = PGGameInstance->GetMaxInventorySize();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find PGGameInstance in InventoryComponent."));
		MaxInventorySize = 5;
	}
	InventoryItems.SetNum(MaxInventorySize);

	for (int32 i = 0; i < MaxInventorySize; i++)
		InventoryItems[i].ItemData = nullptr;

	// Set player character.
	PlayerCharacter = Cast<APGPlayerCharacter>(GetOwner());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find PlayerCharacter in InventoryComponent."));
	}
}

void UPGInventoryComponent::ChangeCurrentInventoryIndex(int32 NewInventoryIndex)
{
	if (NewInventoryIndex >= MaxInventorySize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Changing item index is over the idx."))
			return;
	}

	if (NewInventoryIndex == CurrentInventoryIndex)
	{
		UE_LOG(LogTemp, Log, TEXT("Try to change same item index. Nothing happened."));
		return;
	}
	
	// Item index cannot be changed during a HandAction.
	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC is null in PGInventoryComponent"));
		return;
	}

	FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionActivateTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change item index during a HandAction."));
		return;
	}

	//Before changing item, Deactivate Current Item's Ability.
	DeactivateCurrentItemAbility();

	// Set the Change Item montage in player character and activate GA_HandAction to play Change item anim.
	PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Change);

	// Change current inventory index
	SetCurrentInventoryIndex(NewInventoryIndex);

	UE_LOG(LogTemp, Log, TEXT("Change inventory index to %d"), NewInventoryIndex);
	// After HandAction, activate new ability and change mesh on hand at PGPlayerCharacter::EquipCurrentInventoryItem.
}

//Called server to change index, server do handaction ability and change item index, ability and hand meshes.
void UPGInventoryComponent::SetCurrentInventoryIndex_Implementation(int32 NewIndex)
{
	CurrentInventoryIndex = NewIndex;
	UE_LOG(LogTemp, Log, TEXT("Now inventory index is %d"), CurrentInventoryIndex);
}

//Get item into right index of inventory and grant ability.
void UPGInventoryComponent::AddItemToInventory(UPGItemData* ItemData)
{
	// Set the Pick Item montage in player character and activate GA_HandAction to play pick item anim.
	PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);

	//If invectory is full, return.
	if (bInventoryFull)
		return;

	// If currentidx already has item, get the frontest idx of itemArray and place the ItemData to index.
	// If an item is placed in the current index, set the bool to true, grant the ability, and activate it immediately.
	int32 ItemInputIdx = CurrentInventoryIndex;
	bool IsItemCurrentIdx = true;

	if (InventoryItems[ItemInputIdx].ItemData)
	{
		IsItemCurrentIdx = false;
		for (int32 i = 0; i < MaxInventorySize; i++)
		{
			if (!InventoryItems[i].ItemData)
			{
				ItemInputIdx = i;
				break;
			}
		}
	}
	InventoryItems[ItemInputIdx].ItemData = ItemData;

	UE_LOG(LogTemp, Log, TEXT("Get Item %s in %s 's InventoryComponent"), *InventoryItems[ItemInputIdx].ItemData->ItemName.ToString(), *GetOwner()->GetName());

	// Check the leftover inventory size.
	InventoryItemCount++;
	if (InventoryItemCount == MaxInventorySize)
		bInventoryFull = true;

	// Grant ability with given item's ItemAbility.
	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC is null in PGInventoryComponent"));
		return;
	}
	FGameplayAbilitySpec AbilitySpec(ItemData->ItemAbility, 1);
	InventoryItems[ItemInputIdx].ItemAbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
}

void UPGInventoryComponent::ActivateCurrentItemAbility()
{
	if (InventoryItems[CurrentInventoryIndex].ItemData == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Do not Activete current item. There's no item on current item index."));
		return;
	}

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("ActivateCurrentItemAbility only run on server."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Activate current item's ability by spec handle : %s"), *InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle.ToString());

	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC is null in PGInventoryComponent"));
		return;
	}
	AbilitySystemComponent->TryActivateAbility(InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle);
}

void UPGInventoryComponent::DeactivateCurrentItemAbility()
{
	if (InventoryItems[CurrentInventoryIndex].ItemData == nullptr)
		return;

	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC is null in PGInventoryComponent"));
		return;
	}
	AbilitySystemComponent->CancelAbilityHandle(InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle);
}

void UPGInventoryComponent::RemoveCurrentItem()
{
	DeactivateCurrentItemAbility();

	AbilitySystemComponent->ClearAbility(InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle);

	InventoryItems[CurrentInventoryIndex].ItemData = nullptr;

	InventoryItemCount--;
	bInventoryFull = false;

	UE_LOG(LogTemp, Log, TEXT("Remove item and clear ability."));
}

void UPGInventoryComponent::DropCurrentItem(const FVector& DropLocation)
{
	// Check current index item is valid
	if (InventoryItems[CurrentInventoryIndex].ItemData == nullptr)
		return;

	// Play Hand Action : Drop
	PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Drop);
	
	// Spawn Item ( Set collision Thrown, if hit floor change return )
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		APGItemActor* DropItem = World->SpawnActor<APGItemActor>(ItemActor, DropLocation, FRotator::ZeroRotator, SpawnParams);
		
		UE_LOG(LogTemp, Log, TEXT("DropItem is %s /  DropItemMesh : %s"), *DropItem->GetName(), *DropItem->GetItemData()->ItemMesh.GetFName().ToString());
		if (DropItem)
		{
			UE_LOG(LogTemp, Log, TEXT("Item mesh is %s "), *InventoryItems[CurrentInventoryIndex].ItemData->ItemMesh.GetFName().ToString());
			DropItem->InitWithData(InventoryItems[CurrentInventoryIndex].ItemData);
			DropItem->DropItemSpawned();
		}
	}

	// Remove current Item
	RemoveCurrentItem();
}

bool UPGInventoryComponent::IsInventoryFull()
{
	return bInventoryFull;
}

void UPGInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGInventoryComponent, InventoryItems);
	DOREPLIFETIME(UPGInventoryComponent, CurrentInventoryIndex);
}
