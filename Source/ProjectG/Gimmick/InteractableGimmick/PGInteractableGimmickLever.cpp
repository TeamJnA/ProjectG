// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickLever.h"
#include "Level/Room/PGMirrorRoom.h"
#include "Gimmick/InteractableGimmick/Ability/GA_Interact_Lever.h"
#include "Net/UnrealNetwork.h"

APGInteractableGimmickLever::APGInteractableGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractAbility = UGA_Interact_Lever::StaticClass();
}

void APGInteractableGimmickLever::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGInteractableGimmickLever, bIsActivated);
}

void APGInteractableGimmickLever::SetMasterRoom(APGMirrorRoom* InRoom)
{
	MasterRoom = InRoom;
}

bool APGInteractableGimmickLever::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (bIsActivated)
	{
		OutFailureMessage = FText::FromString(TEXT("Already Activated"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Lever] not activated"));
	return Super::CanStartInteraction(InteractingASC, OutFailureMessage);
}

FInteractionInfo APGInteractableGimmickLever::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, 1.0f);
}

void APGInteractableGimmickLever::ActivateLever()
{
	if (HasAuthority() && !bIsActivated)
	{
		bIsActivated = true;
		if (APGMirrorRoom* Room = MasterRoom.Get())
		{
			Room->SolveGimmick();
		}

		Multicast_ActivatePhysics();
	}
}

void APGInteractableGimmickLever::Multicast_ActivatePhysics_Implementation()
{
	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	StaticMesh->SetCollisionProfileName(TEXT("Item"));
	StaticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	StaticMesh->AddImpulse(GetActorRightVector() * -100.0f, NAME_None, true);

	TWeakObjectPtr<UStaticMeshComponent> WeakMesh(StaticMesh);
	FTimerHandle FreezeTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(FreezeTimerHandle, [WeakMesh]()
	{
		if (WeakMesh.IsValid())
		{
			WeakMesh->SetSimulatePhysics(false);
		}
	}, 3.0f, false);
}
