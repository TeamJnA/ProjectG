// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"

#include "AbilitySystemComponent.h"

// Sets default values
APGInteractableGimmickBase::APGInteractableGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
}

TSubclassOf<UGameplayAbility> APGInteractableGimmickBase::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGInteractableGimmickBase::HighlightOn() const
{
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(true);
	}
}

void APGInteractableGimmickBase::HighlightOff() const
{
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}

FInteractionInfo APGInteractableGimmickBase::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, 1.0f);
}

bool APGInteractableGimmickBase::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (!InteractAbility)
	{
		OutFailureMessage = FailReasonText;
		return false;
	}
	return true;
}

