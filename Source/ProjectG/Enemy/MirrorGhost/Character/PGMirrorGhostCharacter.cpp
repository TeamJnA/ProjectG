// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/MirrorGhost/Character/PGMirrorGhostCharacter.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerController.h"
#include "Character/Component/PGCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Type/PGPhotoTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


APGMirrorGhostCharacter::APGMirrorGhostCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

bool APGMirrorGhostCharacter::IsPhotographable() const
{
	if (!TargetPlayer)
	{
		return false;
	}

	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!LocalPC)
	{
		return false;
	}

	return LocalPC == TargetPlayer->GetController();
}

FPhotoSubjectInfo APGMirrorGhostCharacter::GetPhotoSubjectInfo() const
{
	return FPhotoSubjectInfo(PhotoID::MirrorGhost, 150);
}

FVector APGMirrorGhostCharacter::GetPhotoTargetLocation() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
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
	UE_LOG(LogTemp, Log, TEXT("[MirrorGhost] BeginPlay"));

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void APGMirrorGhostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGMirrorGhostCharacter, bIsFrozen);
	DOREPLIFETIME(APGMirrorGhostCharacter, TargetPlayer);
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

		OnRep_TargetPlayer();
	}
}

void APGMirrorGhostCharacter::OnRep_TargetPlayer()
{
	if (TargetPlayer && TargetPlayer->IsLocallyControlled())
	{
		if (UPGCameraComponent* CameraComp = TargetPlayer->GetCameraComponent())
		{
			if (CameraComp->IsInCameraMode())
			{
				SetCameraModeVisible(true);
			}
		}
	}

	UpdateVisibility();
}

void APGMirrorGhostCharacter::UpdateVisibility()
{
	bool bShouldBeVisible = true;

	if (HasAuthority())
	{
		const APlayerController* HostPC = GetWorld()->GetFirstPlayerController();
		if (HostPC && TargetPlayer && HostPC != TargetPlayer->GetController())
		{
			bShouldBeVisible = false;
		}
	}

	GetMesh()->SetVisibility(bShouldBeVisible);
}

void APGMirrorGhostCharacter::SetCameraModeVisible(bool bVisible)
{
	if (!MirrorGhostMID)
	{
		if (GetMesh() && GetMesh()->GetNumMaterials() > 0)
		{
			MirrorGhostMID = GetMesh()->CreateDynamicMaterialInstance(0);
		}
	}

	if (MirrorGhostMID)
	{
		MirrorGhostMID->SetScalarParameterValue(FName("CameraModeVisible"), bVisible ? 1.0f : 0.0f);
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
	if (!TargetPlayer || !AttackEffectClass)
	{
		Destroy();
		return;
	}

	APGPlayerController* TargetPC = Cast<APGPlayerController>(TargetPlayer->GetController());
	if (TargetPC)
	{
		TargetPC->Client_DisplayJumpscare(MirrorGhostJumpscareTexture);
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
