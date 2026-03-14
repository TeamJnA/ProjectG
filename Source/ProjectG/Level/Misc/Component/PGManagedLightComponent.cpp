// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/Component/PGManagedLightComponent.h"
#include "Components/LightComponent.h"

UPGManagedLightComponent::UPGManagedLightComponent()
{
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);

	SetGenerateOverlapEvents(true);

	SphereRadius = 100.0f;
}

void UPGManagedLightComponent::BeginPlay()
{
	Super::BeginPlay();

	USceneComponent* ParentComp = GetAttachParent();
	if (!ParentComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ManagedLight::BeginPlay: [%s] no parent!"), *GetOwner()->GetName());
		return;
	}

	TargetLight = Cast<ULightComponent>(ParentComp);
	if (!TargetLight)
	{
		UE_LOG(LogTemp, Error, TEXT("ManagedLight::BeginPlay: [%s] Parent is not LightComponent!"),
			*GetOwner()->GetName(), *ParentComp->GetName());
		return;
	}

	OriginalIntensity = TargetLight->Intensity;
	TargetIntensity = OriginalIntensity;

	UE_LOG(LogTemp, Log, TEXT("ManagedLight::BeginPlay: Success. Controlling Light: %s (Intensity: %f)"), *TargetLight->GetName(), OriginalIntensity);
}

void UPGManagedLightComponent::FadeOut()
{
	if (bPermanentOff || !TargetLight)
	{
		return;
	}

	if (TargetIntensity != 0.0f)
	{
		TargetIntensity = 0.0f;
		ManageFadeTimer();
	}
}

void UPGManagedLightComponent::FadeIn()
{
	if (bPermanentOff || !TargetLight)
	{
		return;
	}

	if (TargetIntensity != OriginalIntensity)
	{
		TargetIntensity = OriginalIntensity;
		ManageFadeTimer();
	}
}

void UPGManagedLightComponent::ManageFadeTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle,
		this,
		&UPGManagedLightComponent::UpdateFade,
		FadeUpdateRate,
		true
	);
}

void UPGManagedLightComponent::UpdateFade()
{
	if (bPermanentOff || !TargetLight)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
		return;
	}

	const float CurrentIntensity = TargetLight->Intensity;
	if (FMath::IsNearlyEqual(CurrentIntensity, TargetIntensity, 0.1f))
	{
		TargetLight->SetIntensity(TargetIntensity);
		GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
	else
	{
		const float NewIntensity = FMath::FInterpTo(CurrentIntensity, TargetIntensity, FadeUpdateRate, LightFadeSpeed);
		TargetLight->SetIntensity(NewIntensity);
	}
}

void UPGManagedLightComponent::PowerOff()
{
	bPermanentOff = true;
	PowerOffStep = 0;
	PowerOffSequence();
}

void UPGManagedLightComponent::PowerOffSequence()
{
	if (!TargetLight)
	{
		return;
	}

	switch (PowerOffStep)
	{
		case 0:
		{
			TargetLight->SetIntensity(0.0f);
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedLightComponent::PowerOffSequence, 0.08f, false);
			break;
		}

		case 1:
		{
			TargetLight->SetIntensity(OriginalIntensity * 1.0f);
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedLightComponent::PowerOffSequence, 0.4f, false);
			break;
		}

		case 2:
		{
			TargetLight->SetIntensity(0.0f);
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedLightComponent::PowerOffSequence, 0.1f, false);
			break;
		}

		case 3:
		{
			TargetLight->SetIntensity(OriginalIntensity * 0.6f);
			PowerOffStep++;
			GetWorld()->GetTimerManager().SetTimer(PowerOffTimerHandle, this, &UPGManagedLightComponent::PowerOffSequence, 0.3f, false);
			break;
		}

		case 4:
		{
			TargetLight->SetIntensity(0.0f);
			TargetIntensity = 0.0f;
			break;
		}
	}
}
