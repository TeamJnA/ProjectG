// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/Component/PGManagedEmissiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"

UPGManagedEmissiveComponent::UPGManagedEmissiveComponent()
{
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);
	
	SetGenerateOverlapEvents(true);

	SphereRadius = 100.0f;
}

void UPGManagedEmissiveComponent::BeginPlay()
{
	Super::BeginPlay();

	USceneComponent* ParentComp = GetAttachParent();
	if (!ParentComp)
	{
		UE_LOG(LogTemp, Error, TEXT("Emissive::BeginPlay: [%s] No Parent"), *GetOwner()->GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Emissive::BeginPlay: [%s] Attached to Parent: %s (Class: %s)"),
		*GetOwner()->GetName(), *ParentComp->GetName(), *ParentComp->GetClass()->GetName());

	UMeshComponent* TargetMesh = nullptr;
	TargetMesh = Cast<UMeshComponent>(ParentComp);
	if (TargetMesh)
	{
		UE_LOG(LogTemp, Log, TEXT("Emissive::BeginPlay: Found Mesh first cast: %s"), *TargetMesh->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Emissive::BeginPlay: [%s] failed final check."),
			*GetOwner()->GetName(), *ParentComp->GetName());
		return;
	}

	ManagedMaterials.Empty();
	for (int32 i = 0; i < TargetMesh->GetNumMaterials(); ++i)
	{
		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(TargetMesh->GetMaterial(i));
		if (!MID)
		{
			MID = TargetMesh->CreateAndSetMaterialInstanceDynamic(i);
		}

		if (MID)
		{
			float OriginalValue;
			if (MID->GetScalarParameterValue(EmissiveParamName, OriginalValue))
			{
				ManagedMaterials.Emplace(MID, OriginalValue);
			}
		}
	}

	if (ManagedMaterials.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Emissive::BeginPlay: Success. Cached %d Materials on %s."), ManagedMaterials.Num(), *TargetMesh->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Emissive::BeginPlay: Mesh(%s) found, but no '%s' parameter in materials."), *TargetMesh->GetName(), *EmissiveParamName.ToString());
	}
}

void UPGManagedEmissiveComponent::FadeOut()
{
	if (bPermanentOff)
	{
		return;
	}

	bool bNeedsTimer = false;
	for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
	{
		if (MatInfo.MID)
		{
			MatInfo.TargetValue = 0.0f;
			bNeedsTimer = true;
		}
	}

	if (bNeedsTimer)
	{
		UE_LOG(LogTemp, Log, TEXT("Emissive::FadeOut: [%s]"), *GetOwner()->GetName());
		ManageFadeTimer();
	}
}

void UPGManagedEmissiveComponent::FadeIn()
{
	if (bPermanentOff)
	{
		return;
	}

	bool bNeedsTimer = false;
	for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
	{
		if (MatInfo.MID)
		{
			MatInfo.TargetValue = MatInfo.OriginalValue;
			bNeedsTimer = true;
		}
	}

	if (bNeedsTimer)
	{
		UE_LOG(LogTemp, Log, TEXT("Emissive::FadeIn: [%s]"), *GetOwner()->GetName());
		ManageFadeTimer();
	}
}

void UPGManagedEmissiveComponent::ManageFadeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle,
		this,
		&UPGManagedEmissiveComponent::UpdateFade,
		FadeUpdateRate,
		true
	);
}

void UPGManagedEmissiveComponent::UpdateFade()
{
	if (bPermanentOff)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
		return;
	}

	int32 FadingCount = 0;
	for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
	{
		if (UMaterialInstanceDynamic* MID = MatInfo.MID)
		{
			float CurrentValue;
			MID->GetScalarParameterValue(EmissiveParamName, CurrentValue);
			if (FMath::IsNearlyEqual(CurrentValue, MatInfo.TargetValue, 0.01f))
			{
				MID->SetScalarParameterValue(EmissiveParamName, MatInfo.TargetValue);
			}
			else
			{
				const float NewValue = FMath::FInterpTo(CurrentValue, MatInfo.TargetValue, FadeUpdateRate, LightFadeSpeed);
				MID->SetScalarParameterValue(EmissiveParamName, NewValue);
				FadingCount++;
			}
		}
	}

	if (FadingCount == 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
}

void UPGManagedEmissiveComponent::PowerOff()
{
	bPermanentOff = true;
	PowerOffStep = 0;
	PowerOffSequence();
}

void UPGManagedEmissiveComponent::PowerOffSequence()
{
	switch (PowerOffStep)
	{
		case 0:
		{
			for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
			{
				if (MatInfo.MID) 
				{
					MatInfo.MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				}
			}
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedEmissiveComponent::PowerOffSequence, 0.08f, false);
			break;
		}

		case 1:
		{
			for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
			{
				if (MatInfo.MID) 
				{
					MatInfo.MID->SetScalarParameterValue(EmissiveParamName, MatInfo.OriginalValue * 0.6f);
				}
			}
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedEmissiveComponent::PowerOffSequence, 0.4f, false);
			break;
		}

		case 2:
		{
			for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
			{
				if (MatInfo.MID)
				{
					MatInfo.MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				}
			}
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedEmissiveComponent::PowerOffSequence, 0.1f, false);
			break;
		}

		case 3:
		{
			for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
			{
				if (MatInfo.MID)
				{
					MatInfo.MID->SetScalarParameterValue(EmissiveParamName, MatInfo.OriginalValue * 0.3f);
				}
			}
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedEmissiveComponent::PowerOffSequence, 0.3f, false);
			break;
		}

		case 4:
		{
			for (FManagedMaterialInfo& MatInfo : ManagedMaterials)
			{
				if (MatInfo.MID)
				{
					MatInfo.MID->SetScalarParameterValue(EmissiveParamName, 0.0f);
				}
				MatInfo.TargetValue = 0.0f;
			}
			break;
		}
	}
}