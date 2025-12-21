// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/PGInventoryComponent.h"

#include "Game/PGAdvancedFriendsGameInstance.h"
#include "Character/PGPlayerCharacter.h"
#include "Type/CharacterTypes.h"
#include "AbilitySystemComponent.h"
#include "Item/PGItemActor.h"
#include "Item/PGItemData.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Character/Component/PGSoundManagerComponent.h"

DEFINE_LOG_CATEGORY(LogInventory);

// Sets default values for this component's properties
UPGInventoryComponent::UPGInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bInventoryFull = false;
	InventoryItemCount = 0;
	CurrentInventoryIndex = 0;
	MaxInventorySize = 5;

	bPrevHeldItemFlag = false;

	SetIsReplicatedByDefault(true);

	static ConstructorHelpers::FClassFinder<APGItemActor> ItemActorRef(TEXT("/Game/ProjectG/Items/BP_PGItemActor.BP_PGItemActor_C"));
	if (ItemActorRef.Class)
	{
		ItemActor = ItemActorRef.Class;
	}

	PickItemSound = FName(TEXT("PLAYER_Interact_PickItem"));
}

void UPGInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UPGAdvancedFriendsGameInstance* PGGameInstance = Cast<UPGAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		MaxInventorySize = PGGameInstance->GetMaxInventorySize();
	}
	else
	{
		ensureMsgf(false, TEXT("Cannot find UPGAdvancedFriendsGameInstance in UPGInventoryComponent::BeginPlay."));
	}

	InventoryItems.SetNum(MaxInventorySize);

	for (int32 i = 0; i < MaxInventorySize; i++)
	{
		InventoryItems[i].ItemData = nullptr;
	}

	PlayerCharacter = Cast<APGPlayerCharacter>(GetOwner());
	check(PlayerCharacter);
}

void UPGInventoryComponent::ChangeCurrentInventoryIndex(const int32 NewInventoryIndex)
{
	check(NewInventoryIndex < MaxInventorySize);

	if (NewInventoryIndex == CurrentInventoryIndex)
	{
		UE_LOG(LogInventory, Log, TEXT("Try to change same item index. Nothing happened."));
		return;
	}
	
	// Item index cannot be changed during HandAction.
	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogInventory, Warning, TEXT("ASC is null in PGInventoryComponent"));
		return;
	}

	const FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionActivateTag))
	{
		UE_LOG(LogInventory, Log, TEXT("Cannot change item index during a HandAction."));
		return;
	}

	//Before changing item, Deactivate Current Item's Ability.
	DeactivateItemAbility(CurrentInventoryIndex);

	// Current Inventory index나 New inventory index에 아이템이 있으면 액션 및 사운드 재생 ( 둘 다 비었을 경우 재생 X )
	if (InventoryItems[CurrentInventoryIndex].ItemData || InventoryItems[NewInventoryIndex].ItemData)
	{
		// Play HandAction with Change anim.
		// After HandAction, activate new ability and change mesh on hand at PGPlayerCharacter::EquipCurrentInventoryItem.
		PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Change);

		if (UPGSoundManagerComponent* SoundManagerComp = GetOwner()->FindComponentByClass<UPGSoundManagerComponent>())
		{
			SoundManagerComp->TriggerSoundWithNoise(PickItemSound, GetOwner()->GetActorLocation());
		}
	}

	SetCurrentInventoryIndex(NewInventoryIndex);

	// Broadcast to InventoryWidget
	if (OnCurrentSlotIndexChanged.IsBound())
	{
		OnCurrentSlotIndexChanged.Broadcast(NewInventoryIndex);
	}

	UE_LOG(LogInventory, Log, TEXT("Change inventory index to %d"), NewInventoryIndex);
}

void UPGInventoryComponent::SetCurrentInventoryIndex_Implementation(const int32 NewIndex)
{
	CurrentInventoryIndex = NewIndex;
	UE_LOG(LogInventory, Log, TEXT("Now inventory index is %d"), CurrentInventoryIndex);
}

//Get item into right index of inventory and grant ability.
void UPGInventoryComponent::AddItemToInventory(UPGItemData* ItemData)
{
	// Play HandAction with Pick anim.
	PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Pick);

	if (bInventoryFull)
	{
		UE_LOG(LogInventory, Log, TEXT("Inventory is full. Cannot pick up more item."));
		return;
	}

	// If CurrentInventoryIndex already has item, get the frontest empty idx and place the ItemData to the empty index.
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

	UE_LOG(LogInventory, Log, TEXT("Get Item %s in %s 's InventoryComponent"), *InventoryItems[ItemInputIdx].ItemData->ItemName.ToString(), *GetOwner()->GetName());

	InventoryItemCount++;
	if (InventoryItemCount == MaxInventorySize)
	{
		bInventoryFull = true;
	}

	// Grant ability with given item's ItemAbility.
	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogInventory, Warning, TEXT("ASC is null in PGInventoryComponent::AddItemToInventory"));
		return;
	}
	const FGameplayAbilitySpec AbilitySpec(ItemData->ItemAbility, 1);
	InventoryItems[ItemInputIdx].ItemAbilitySpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);

	// Broadcast to Inventory Widget
	OnInventoryItemUpdate.Broadcast(InventoryItems);

	// Play sound
	if (UPGSoundManagerComponent* SoundManagerComp = GetOwner()->FindComponentByClass<UPGSoundManagerComponent>())
	{
		SoundManagerComp->TriggerSoundWithNoise(PickItemSound, GetOwner()->GetActorLocation());
	}
}

/*
* On client
* inventory items update (add/delete) -> update inventory widget
*/
void UPGInventoryComponent::OnRep_InventoryItems()
{
	OnInventoryItemUpdate.Broadcast(InventoryItems);
}

void UPGInventoryComponent::OnRep_HeldItemFlag()
{
	OnItemHeldStateChanged.Broadcast(bPrevHeldItemFlag);
}

void UPGInventoryComponent::ActivateCurrentItemAbility()
{
	if (InventoryItems[CurrentInventoryIndex].ItemData == nullptr)
	{
		return;
	}

	// ActivateCurrentItemAbility only run on server.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UE_LOG(LogInventory, Log, TEXT("Activate current item's ability by spec handle : %s"), *InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle.ToString());

	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogInventory, Warning, TEXT("ASC is null in PGInventoryComponent::ActivateCurrentItemAbility"));
		return;
	}

	AbilitySystemComponent->TryActivateAbility(InventoryItems[CurrentInventoryIndex].ItemAbilitySpecHandle);

	Server_CheckHeldItemChanged();
}

void UPGInventoryComponent::DeactivateItemAbility(const int32 DeactivateItemIndex)
{
	if (InventoryItems[DeactivateItemIndex].ItemData == nullptr)
	{
		return;
	}

	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogInventory, Warning, TEXT("ASC is null in PGInventoryComponent::DeactivateCurrentItemAbility"));
		return;
	}

	AbilitySystemComponent->CancelAbilityHandle(InventoryItems[DeactivateItemIndex].ItemAbilitySpecHandle);
}

void UPGInventoryComponent::DropItemByIndex(const FVector DropLocation, const FRotator DropRotation, const int32 DropItemIndex)
{
	check(DropItemIndex < MaxInventorySize && DropItemIndex >= 0);

	if (InventoryItems[DropItemIndex].ItemData == nullptr)
	{
		return;
	}

	// Play HandAction with Drop anim.
	PlayerCharacter->PlayHandActionAnimMontage(EHandActionMontageType::Drop);

	// Spawn Item
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		UE_LOG(LogInventory, Log, TEXT("Spawned item to drop."));
		APGItemActor* DropItem = World->SpawnActor<APGItemActor>(ItemActor, DropLocation, FRotator::ZeroRotator, SpawnParams);

		if (DropItem)
		{
			DropItem->InitWithData(InventoryItems[DropItemIndex].ItemData);
			DropItem->DropItemSpawned(DropRotation);
		}
	}

	RemoveItem(DropItemIndex);
}

void UPGInventoryComponent::DropCurrentItem(const FVector DropLocation, const FRotator DropRotation)
{
	DropItemByIndex(DropLocation, DropRotation, CurrentInventoryIndex);
}

void UPGInventoryComponent::RemoveItem(const int32 RemoveItemIndex)
{
	AbilitySystemComponent = PlayerCharacter->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogInventory, Warning, TEXT("ASC is null in PGInventoryComponent::RemoveCurrentItem. Cannot remove item ability."));
		return;
	}

	DeactivateItemAbility(RemoveItemIndex);

	AbilitySystemComponent->ClearAbility(InventoryItems[RemoveItemIndex].ItemAbilitySpecHandle);

	InventoryItems[RemoveItemIndex].ItemData = nullptr;

	InventoryItemCount--;
	bInventoryFull = false;

	UE_LOG(LogInventory, Log, TEXT("Remove item and clear ability."));

	Server_CheckHeldItemChanged();

	/*
	* Broadcast to Inventory Widget
	*/
	OnInventoryItemUpdate.Broadcast(InventoryItems);
}

void UPGInventoryComponent::RemoveCurrentItem()
{
	RemoveItem(CurrentInventoryIndex);
}

void UPGInventoryComponent::DropAllItems(const FVector DropLocation)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UE_LOG(LogInventory, Log, TEXT("Drop all items"));

	for (int32 i = 0; i < MaxInventorySize; i++)
	{
		if (InventoryItems[i].ItemData != nullptr)
		{
			FRotator DropRotation(0.0f, 72.0f * i, 0.0f);
			DropItemByIndex(DropLocation, DropRotation, i);
		}
	}
}

void UPGInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPGInventoryComponent, InventoryItems);
	DOREPLIFETIME(UPGInventoryComponent, CurrentInventoryIndex);
	DOREPLIFETIME(UPGInventoryComponent, bPrevHeldItemFlag);
}

TObjectPtr<UPGItemData> UPGInventoryComponent::GetCurrentItemMesh() const
{
	if (InventoryItems[CurrentInventoryIndex].ItemData)
	{
		return InventoryItems[CurrentInventoryIndex].ItemData;
	}
	else
	{
		return nullptr;
	}
}

void UPGInventoryComponent::Server_CheckHeldItemChanged_Implementation()
{
	if ( bPrevHeldItemFlag && InventoryItems[CurrentInventoryIndex].ItemData == nullptr)
	{
		bPrevHeldItemFlag = false;
		OnItemHeldStateChanged.Broadcast(false);
	}
	else if ( !bPrevHeldItemFlag && InventoryItems[CurrentInventoryIndex].ItemData)
	{
		bPrevHeldItemFlag = true;
		OnItemHeldStateChanged.Broadcast(true);
	}
}

