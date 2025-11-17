// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/Component/PGManagedLightComponent.h"
#include "Components/LightComponent.h"

UPGManagedLightComponent::UPGManagedLightComponent()
{
	SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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
	if (!TargetLight)
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
	if (!TargetLight)
	{
		return;
	}

	if (TargetIntensity != OriginalIntensity)
	{
		TargetIntensity = OriginalIntensity;
		ManageFadeTimer();
	}
}

void UPGManagedLightComponent::UpdateFade()
{
	if (!TargetLight)
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
