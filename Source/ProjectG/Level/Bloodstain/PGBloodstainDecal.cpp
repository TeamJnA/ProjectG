// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Bloodstain/PGBloodstainDecal.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APGBloodstainDecal::APGBloodstainDecal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(2.0f);

	DecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComponent"));
	RootComponent = DecalComponent;
	DecalComponent->DecalSize = FVector(20.0f, 100.0f, 100.0f);
}

void APGBloodstainDecal::InitializeDecal(const FVector& NewDecalSize, UMaterialInterface* NewDecalMaterial)
{
	if (HasAuthority())
	{
		TargetDecalSize = NewDecalSize;
		OnRep_TargetDecalSize();

		if (NewDecalMaterial)
		{
			TargetDecalMaterial = NewDecalMaterial;
			OnRep_TargetDecalMaterial();
		}

		// 이후 상태가 변하지 않으므로 최초 1회 복제 후 휴면
		SetNetDormancy(DORM_DormantAll);
	}
}

void APGBloodstainDecal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGBloodstainDecal, TargetDecalSize);
	DOREPLIFETIME(APGBloodstainDecal, TargetDecalMaterial);
}

void APGBloodstainDecal::OnRep_TargetDecalSize()
{
	if (DecalComponent)
	{
		DecalComponent->DecalSize = TargetDecalSize;
		DecalComponent->MarkRenderStateDirty();
	}
}

void APGBloodstainDecal::OnRep_TargetDecalMaterial()
{
	if (DecalComponent && TargetDecalMaterial)
	{
		DecalComponent->SetDecalMaterial(TargetDecalMaterial);
	}
}
