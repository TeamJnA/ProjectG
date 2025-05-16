// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Ability/GA_Interact_Item.h"
#include "AbilitySystemComponent.h"
#include "Interface/ItemInteractInterface.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"

#include "Item/PGItemData.h"

UGA_Interact_Item::UGA_Interact_Item()
{
    //Add Event trigger
    FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Interact"));

    //Block ability when using hands.
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked")));

    FAbilityTriggerData TriggerData;
    TriggerData.TriggerTag = TriggerTag;
    TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

    AbilityTriggers.Add(TriggerData);

    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Interact_Item::ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, 
    const FGameplayEventData* TriggerEventData)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    PG_CHECK_VALID_INTERACT(ASC);

    // Get the inventory component from AvatarActor
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    PG_CHECK_VALID_INTERACT(AvatarActor);

    if (IsValid(TriggerEventData->Target))
        UE_LOG(LogTemp, Log, TEXT("Activate interact %s ability to %s"), *TriggerEventData->Target.GetFName().ToString(), *GetOwningActorFromActorInfo()->GetName());

    UPGInventoryComponent* PGInventory = AvatarActor->FindComponentByClass<UPGInventoryComponent>();
    PG_CHECK_VALID_INTERACT(PGInventory);

    // If player inventory is full, return.
    if (PGInventory->IsInventoryFull())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    APGPlayerCharacter* PGCharacter = Cast<APGPlayerCharacter>(AvatarActor);
    PG_CHECK_VALID_INTERACT(PGCharacter);

    // Check if the target actor in the character matches the one in the event data
    AActor* TargetActor = PGCharacter->GetInteractionTargetActor();
    if(TargetActor != TriggerEventData->Target.Get()){
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        UE_LOG(LogTemp, Log, TEXT("TargetActor does not match the payload target"));
        return;
    }
    
    // Get item data from the item actor. If successful, destroy the item.
    IItemInteractInterface* ItemInteractInterface = Cast<IItemInteractInterface>(TargetActor);
    PG_CHECK_VALID_INTERACT(ItemInteractInterface);

    UPGItemData* ItemData = ItemInteractInterface->GetItemData();
    PG_CHECK_VALID_INTERACT(ItemData);

    UE_LOG(LogTemp, Log, TEXT("Success to get %s"), *ItemData->ItemName.ToString());

    TargetActor->Destroy();

    // Add item to inventory
    PGInventory->AddItemToInventory(ItemData);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

//When the ability end, need to clear the ability. This ability is temporary.
void UGA_Interact_Item::EndAbility(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo, 
    bool bReplicateEndAbility, 
    bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    FGameplayAbilitySpecHandle HandleToRemove = GetCurrentAbilitySpecHandle();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    ASC->ClearAbility(HandleToRemove);
}