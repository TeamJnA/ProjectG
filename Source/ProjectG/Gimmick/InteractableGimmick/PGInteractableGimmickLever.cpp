// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickLever.h"
#include "Level/Room/PGMirrorRoom.h"
#include "Gimmick/InteractableGimmick/Ability/GA_Interact_Lever.h"
#include "Net/UnrealNetwork.h"

#include "GameFramework/GameModeBase.h"
#include "Sound/PGSoundManager.h"
#include "Interface/SoundManagerInterface.h"

APGInteractableGimmickLever::APGInteractableGimmickLever()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractAbility = UGA_Interact_Lever::StaticClass();

	StaticMesh->SetRenderCustomDepth(true);
	StaticMesh->SetCustomDepthStencilValue(1);

	GlassPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassPlane"));
	GlassPlane->SetupAttachment(StaticMesh);
	GlassPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APGInteractableGimmickLever::BeginPlay()
{
	Super::BeginPlay();

	if (StaticMesh && StaticMesh->GetNumMaterials() > 1)
	{
		FrameMID = StaticMesh->CreateDynamicMaterialInstance(0);
		PaintMID = StaticMesh->CreateDynamicMaterialInstance(1);
	}

	if (GlassPlane && GlassPlane->GetNumMaterials() > 0)
	{
		CrackMID = GlassPlane->CreateDynamicMaterialInstance(0);
	}
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

FInteractionInfo APGInteractableGimmickLever::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, 1.0f);
}

FText APGInteractableGimmickLever::GetInteractionText() const
{
	return bIsActivated ? FText::GetEmpty() : BreakText;
}

bool APGInteractableGimmickLever::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
{
	if (bIsActivated)
	{
		OutFailurePrompt.Icon = nullptr;
		OutFailurePrompt.IconSize = FVector2D::ZeroVector;
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[Lever] not activated"));
	return Super::CanStartInteraction(InteractingASC, OutFailurePrompt);
}

void APGInteractableGimmickLever::ActivateLever()
{
	//Play Sound	
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* LeverSoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			LeverSoundManager->PlaySoundForAllPlayers(FrameFallSound, GetActorLocation());
		}
	}

	if (HasAuthority() && !bIsActivated)
	{
		bIsActivated = true;
		if (APGMirrorRoom* Room = MasterRoom.Get())
		{
			Room->SolveGimmick();
		}

		Multicast_OnInteractionComplete();
	}
}

void APGInteractableGimmickLever::UpdateHoldProgress(float Progress)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsActivated)
	{
		return;
	}

	int32 NewStage = -1;
	for (int32 i = 0; i < CrackStageThresholds.Num(); i++)
	{
		if (Progress >= CrackStageThresholds[i])
		{
			NewStage = i;
		}
	}

	if (NewStage != CurrentCrackStage)
	{
		CurrentCrackStage = NewStage;
		Multicast_SetCrackStage(NewStage);
	}
}

void APGInteractableGimmickLever::StopHoldProress()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsActivated)
	{
		return;
	}

	CurrentCrackStage = -1;
	Multicast_SetCrackStage(-1);
}

void APGInteractableGimmickLever::Multicast_SetCrackStage_Implementation(int32 Stage)
{
	if (!CrackMID) 
	{
		return;
	}

	float CrackValue = 0.0f;
	if (Stage >= 0 && Stage < CrackStageValues.Num())
	{
		CrackValue = CrackStageValues[Stage];
	}

	CrackMID->SetScalarParameterValue(CrackScale, CrackValue);
}

void APGInteractableGimmickLever::Multicast_OnInteractionComplete_Implementation()
{
	SelfHighlightOff();
	StaticMesh->SetCollisionProfileName(TEXT("Item"));
	StaticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	if (FrameMID)
	{
		FrameMID->SetScalarParameterValue(ShakeParameterName, ShakeIntensity);
	}

	if (PaintMID)
	{
		PaintMID->SetScalarParameterValue(ShakeParameterName, ShakeIntensity);
		PaintMID->SetScalarParameterValue(PaintFadeParameterName, 1.0f);
	}

	if (CrackMID)
	{
		CrackMID->SetScalarParameterValue(ShakeParameterName, ShakeIntensity);
		CrackMID->SetScalarParameterValue(HoleAmountParamName, 1.0f);
	}

	FTimerHandle ShakeEffectTimerHandle;
	GetWorldTimerManager().SetTimer(ShakeEffectTimerHandle, this, &APGInteractableGimmickLever::DisableShakeEffect, ShakeDuration, false);
}

void APGInteractableGimmickLever::DisableShakeEffect()
{
	if (FrameMID && PaintMID && CrackMID)
	{
		FrameMID->SetScalarParameterValue(ShakeParameterName, 0.0f);
		PaintMID->SetScalarParameterValue(ShakeParameterName, 0.0f);
		CrackMID->SetScalarParameterValue(ShakeParameterName, 0.0f);
	}
}
