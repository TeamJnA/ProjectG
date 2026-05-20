// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"

#include "GameFramework/GameModeBase.h"
#include "Sound/PGSoundManager.h"
#include "Interface/SoundManagerInterface.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

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

void APGInteractableGimmickBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitSoundManager();
	}
}

void APGInteractableGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGInteractableGimmickBase, SoundManager);
}

void APGInteractableGimmickBase::PlayLocalSound(FName _SoundName, FVector _SoundLocation)
{
	if (SoundManager)
	{
		SoundManager->PlaySoundLocally(_SoundName, _SoundLocation);
	}
}

void APGInteractableGimmickBase::GimmickInteract()
{
}

void APGInteractableGimmickBase::InitSoundManager()
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		SoundManager = GameModeSoundManagerInterface->GetSoundManager();
		if (SoundManager)
		{
			UE_LOG(LogTemp, Log, TEXT("Init SoundManager Completely [%s]"), *GetNameSafe(this));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Init SoundManager Failed. Cannot find SoundManager in interface [%s]"), *GetNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Init SoundManager Failed. Cannot find soundmanagerInterface [%s]"), *GetNameSafe(this));
	}
}

TSubclassOf<UGameplayAbility> APGInteractableGimmickBase::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGInteractableGimmickBase::HighlightOn() const
{
	/*
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(true);
		StaticMesh->SetCustomDepthStencilValue(0);
	}
	*/
}

void APGInteractableGimmickBase::HighlightOff() const
{
	/*
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
	*/
}

FInteractionInfo APGInteractableGimmickBase::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, 1.0f);
}

bool APGInteractableGimmickBase::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
{
	if (!InteractAbility)
	{
		OutFailurePrompt.Icon = nullptr;
		OutFailurePrompt.IconSize = FVector2D::ZeroVector;
		return false;
	}
	return true;
}

void APGInteractableGimmickBase::SelfHighlightOn()
{
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(true);
		StaticMesh->SetCustomDepthStencilValue(0);
	}
}

void APGInteractableGimmickBase::SelfHighlightOff()
{
	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(false);
	}
}
