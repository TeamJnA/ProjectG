// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGSwingProp.h"
#include "Character/PGPlayerCharacter.h"
#include "Character/Component/PGSoundManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


APGSwingProp::APGSwingProp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetNotifyRigidBodyCollision(true);
	Mesh->SetGenerateOverlapEvents(false);

	bReplicates = true;
	SetReplicateMovement(false);
	bAlwaysRelevant = true;
}

void APGSwingProp::BeginPlay()
{
	Super::BeginPlay();
	
	InitialRotation = GetActorRotation();
	if (HasAuthority())
	{
		Mesh->OnComponentHit.AddDynamic(this, &APGSwingProp::OnHit);
	}
}

void APGSwingProp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGSwingProp, MaterialIndex);
}

void APGSwingProp::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this)
	{
		return;
	}

	APGPlayerCharacter* HitPlayer = Cast<APGPlayerCharacter>(OtherActor);
	if (!HitPlayer)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (LastHitTime > 0.0f && (Now - LastHitTime) < HitCooldown)
	{
		return;
	}
	LastHitTime = Now;

	FVector HitDirection = GetActorLocation() - OtherActor->GetActorLocation();
	HitDirection.Z = 0.0f;
	if (!HitDirection.Normalize())
	{
		return;
	}

	const FVector NewAxis = FVector::CrossProduct(FVector::UpVector, -HitDirection).GetSafeNormal();
	const float NewAmplitude = FMath::Min(CurrentAmplitude + MaxSwingAngle, MaxSwingAngle * 1.5f);


	if (UPGSoundManagerComponent* SMComp = HitPlayer->GetSoundManagerComponent())
	{
		SMComp->TriggerSoundWithNoise(HitSoundName, GetActorLocation(), false);
		SMComp->TriggerSoundWithNoise(SqueakSoundName, GetActorLocation(), false);
	}

	Multicast_StartSwing(NewAxis, NewAmplitude);
}

void APGSwingProp::Multicast_StartSwing_Implementation(FVector_NetQuantizeNormal InSwingAxis, float InAmplitude)
{
	StartSwingLocally(InSwingAxis, InAmplitude);
}

void APGSwingProp::StartSwingLocally(const FVector& InSwingAxis, float InAmplitude)
{
	SwingAxis = InSwingAxis;
	CurrentAmplitude = InAmplitude;
	SwingElapsed = 0.0f;
	bIsSwinging = true;

	SetActorTickEnabled(true);
}

void APGSwingProp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsSwinging)
	{
		return;
	}

	SwingElapsed += DeltaTime;

	const float Decay = FMath::Exp(-SwingDamping * SwingElapsed);
	const float Angle = CurrentAmplitude * Decay * FMath::Sin(2.0f * PI * SwingFrequency * SwingElapsed);
	const FQuat SwingQuat = FQuat(SwingAxis, FMath::DegreesToRadians(Angle));
	SetActorRotation(SwingQuat * InitialRotation.Quaternion());

	if (Decay * CurrentAmplitude < 0.05f)
	{
		SetActorRotation(InitialRotation);
		bIsSwinging = false;
		CurrentAmplitude = 0.0f;
		SetActorTickEnabled(false);
	}
}

void APGSwingProp::SetMaterialVariation(int32 InIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	MaterialIndex = InIndex;
	OnRep_MaterialIndex();
}

void APGSwingProp::OnRep_MaterialIndex()
{
	if (!Mesh || !MaterialVariations.IsValidIndex(MaterialIndex))
	{
		return;
	}

	Mesh->SetMaterial(0, MaterialVariations[MaterialIndex]);
}
