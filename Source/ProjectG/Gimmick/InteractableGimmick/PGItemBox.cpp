// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGItemBox.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGInventoryComponent.h"
#include "Item/PGItemData.h"
#include "AbilitySystemComponent.h"


APGItemBox::APGItemBox()
{
	bReplicates = true;
	SetReplicateMovement(false);
}

FInteractionInfo APGItemBox::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Instant);
}

FText APGItemBox::GetInteractionText() const
{
	return TakeText;
}

bool APGItemBox::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
{
	if (!InteractAbility || !ItemToDispense)
	{
		OutFailurePrompt.Icon = nullptr;
		OutFailurePrompt.IconSize = FVector2D::ZeroVector;
		return false;
	}

	if (const AActor* Avatar = InteractingASC ? InteractingASC->GetAvatarActor() : nullptr)
	{
		if (const UPGInventoryComponent* Inventory = Avatar->FindComponentByClass<UPGInventoryComponent>())
		{
			if (Inventory->IsInventoryFull())
			{
				OutFailurePrompt.Icon = nullptr;
				OutFailurePrompt.IconSize = FVector2D::ZeroVector;
				return false;
			}
		}
	}

	return true;
}

void APGItemBox::GimmickInteract(AActor* Investigator)
{
	if (!HasAuthority() || !ItemToDispense)
	{
		return;
	}

	APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(Investigator);
	if (!PlayerCharacter)
	{
		return;
	}

	UPGInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	if (!Inventory || Inventory->IsInventoryFull())
	{
		return;
	}

	Inventory->AddItemToInventory(ItemToDispense);
}
