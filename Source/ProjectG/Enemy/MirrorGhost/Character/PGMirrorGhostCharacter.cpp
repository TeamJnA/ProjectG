// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"
#include "Character/PGPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

APGMirrorGhostCharacter::APGMirrorGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Character"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

bool APGMirrorGhostCharacter::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (!TargetPlayer)
	{
		return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
	}

	const APlayerController* ViewerPC = Cast<APlayerController>(RealViewer);
	if (ViewerPC && ViewerPC == TargetPlayer->GetController())
	{
		return true;
	}
	return false;
}

void APGMirrorGhostCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void APGMirrorGhostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGMirrorGhostCharacter, bIsFrozen);
}

void APGMirrorGhostCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && TargetPlayer)
	{
		UpdateMovement(DeltaTime);
	}
}

void APGMirrorGhostCharacter::SetTargetPlayer(APGPlayerCharacter* InTargetPlayer)
{
	UE_LOG(LogTemp, Log ,TEXT("[MirrorGhost] SetTargetPlayer"));
	TargetPlayer = InTargetPlayer;
	if (InTargetPlayer)
	{
		UE_LOG(LogTemp, Log, TEXT("[MirrorGhost] complete"));
		SetOwner(InTargetPlayer->GetController());
	}
}

void APGMirrorGhostCharacter::UpdateMovement(float DeltaTime)
{
	const bool bLooking = IsPlayerLooking();
	if (bIsFrozen != bLooking)
	{
		bIsFrozen = bLooking;
		OnRep_IsFrozen();
	}

	if (bIsFrozen)
	{
		GetCharacterMovement()->StopMovementImmediately();
		return;
	}

	const float Distance = FVector::Dist2D(GetActorLocation(), TargetPlayer->GetActorLocation());

	if (Distance > AttackDistance)
	{
		const FVector Direction = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);

		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPlayer->GetActorLocation());
		SetActorRotation(FRotator(0.0f, LookAtRot.Yaw, 0.0f));
	}
	else
	{
		JumpscareAndDestroy();
	}
}

bool APGMirrorGhostCharacter::IsPlayerLooking() const
{
	if (!TargetPlayer || !TargetPlayer->GetController())
	{
		return false;
	}

	FVector CameraLoc;
	FRotator CameraRot;
	TargetPlayer->GetController()->GetPlayerViewPoint(CameraLoc, CameraRot);

	const FVector DirToGhost = (GetActorLocation() - CameraLoc).GetSafeNormal();
	const float DotResult = FVector::DotProduct(CameraRot.Vector(), DirToGhost);

	if (DotResult > StopAngleThreshold)
	{
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(TargetPlayer);
		Params.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, GetActorLocation(), ECC_Visibility, Params);

		return !bHit;
	}

	return false;
}

void APGMirrorGhostCharacter::JumpscareAndDestroy()
{
	if (!AttackEffectClass)
	{
		Destroy();
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetPlayer->GetAbilitySystemComponent();
	if (TargetASC)
	{
		FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
		ContextHandle.AddInstigator(this, this);

		FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(AttackEffectClass, 1.0f, ContextHandle);
		if (SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	Destroy();
}

void APGMirrorGhostCharacter::OnRep_IsFrozen()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	MeshComp->bPauseAnims = bIsFrozen;
	MeshComp->GlobalAnimRateScale = bIsFrozen ? 0.0f : 1.0f;
}
