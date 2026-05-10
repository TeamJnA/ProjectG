// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickBonfire.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "NiagaraComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

APGInteractableGimmickBonfire::APGInteractableGimmickBonfire()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	SanityHealAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SanityHealAreaSphere"));
	SanityHealAreaSphere->SetupAttachment(RootComponent);
	SanityHealAreaSphere->SetSphereRadius(500.0f);
	SanityHealAreaSphere->SetGenerateOverlapEvents(true);

	InteractCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollisionBox"));
	InteractCollisionBox->SetupAttachment(RootComponent);

	FireEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireEffect"));
	FireEffect->SetupAttachment(RootComponent);
	FireEffect->bAutoActivate = false;

	FireLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
	FireLight->SetupAttachment(RootComponent);
	FireLight->SetIntensity(0.0f);
}

void APGInteractableGimmickBonfire::BeginPlay()
{
	Super::BeginPlay();

	SelfHighlightOn();

	if (StaticMesh)
	{
		BonfireMID = StaticMesh->CreateDynamicMaterialInstance(0);
		if (BonfireMID)
		{
			BonfireMID->SetScalarParameterValue(FName("EmissiveValue"), 0.0f);
		}
	}

	if (HasAuthority())
	{
		SanityHealAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APGInteractableGimmickBonfire::OnHealAreaBeginOverlap);
		SanityHealAreaSphere->OnComponentEndOverlap.AddDynamic(this, &APGInteractableGimmickBonfire::OnHealAreaEndOverlap);
	}
}

void APGInteractableGimmickBonfire::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGInteractableGimmickBonfire, bIsLit);
}

void APGInteractableGimmickBonfire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentLightIntensity = FMath::FInterpTo(CurrentLightIntensity, TargetLightIntensity, DeltaTime, FadeSpeed);
	CurrentEmissiveValue = FMath::FInterpTo(CurrentEmissiveValue, TargetEmissiveValue, DeltaTime, FadeSpeed);

	if (FireLight)
	{
		FireLight->SetIntensity(CurrentLightIntensity);
	}

	if (BonfireMID)
	{
		BonfireMID->SetScalarParameterValue(FName("EmissiveValue"), CurrentEmissiveValue);
	}

	const bool bLightFinished = FMath::IsNearlyEqual(CurrentLightIntensity, TargetLightIntensity, 1.0f);
	const bool bEmissiveFinished = FMath::IsNearlyEqual(CurrentEmissiveValue, TargetEmissiveValue, 0.1f);
	if (bLightFinished && bEmissiveFinished)
	{
		if (FireLight)
		{
			FireLight->SetIntensity(TargetLightIntensity);
		}

		if (BonfireMID) 
		{
			BonfireMID->SetScalarParameterValue(FName("EmissiveValue"), TargetEmissiveValue);
		}

		SetActorTickEnabled(false);
	}
}

bool APGInteractableGimmickBonfire::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (!bIsLit)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Match"))))
		{
			return true;
		}
	}
	else
	{
		OutFailureMessage = FText::FromString(TEXT("Already fire"));
		return false;
	}

	OutFailureMessage = FText::FromString(TEXT("Cannot fire"));
	return false;
}

void APGInteractableGimmickBonfire::StartBonfire()
{
	if (!HasAuthority() || bIsLit)
	{
		return;
	}

	bIsLit = true;
	OnRep_IsLit();

	// 이미 범위 안에 있는 플레이어 추적, Vignette on
	TArray<AActor*> OverlappingActors;
	SanityHealAreaSphere->GetOverlappingActors(OverlappingActors, APGPlayerCharacter::StaticClass());
	for (AActor* Actor : OverlappingActors)
	{
		if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(Actor))
		{
			PlayersInHealArea.Add(Player);
			Player->Client_SetBonfireVignetteIntensity(VignetteIntensity);
		}
	}

	// Sanity Heal Timer
	GetWorld()->GetTimerManager().SetTimer(SanityHealTimerHandle, this, &APGInteractableGimmickBonfire::OnHealTick, SanityHealInterval, true);
	// Bonfire Duration Timer
	GetWorld()->GetTimerManager().SetTimer(BoneFireDurationTimerHandle, this, &APGInteractableGimmickBonfire::StopBonfire, BonfireDuration, false);
}

void APGInteractableGimmickBonfire::StopBonfire()
{
	if (!HasAuthority() || !bIsLit)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(SanityHealTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BoneFireDurationTimerHandle);

	// 추적 중인 플레이어들 Vignette off
	for (const TWeakObjectPtr<APGPlayerCharacter>& WeakPlayer : PlayersInHealArea)
	{
		if (APGPlayerCharacter* Player = WeakPlayer.Get())
		{
			Player->Client_SetBonfireVignetteIntensity(0.0f);
		}
	}
	PlayersInHealArea.Empty();
	
	bIsLit = false;
	OnRep_IsLit();
}

void APGInteractableGimmickBonfire::OnHealAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bIsLit)
	{
		return;
	}

	APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	if (!PlayersInHealArea.Contains(Player))
	{
		PlayersInHealArea.Add(Player);
		Player->Client_SetBonfireVignetteIntensity(VignetteIntensity);
	}
}

void APGInteractableGimmickBonfire::OnHealAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	if (PlayersInHealArea.Contains(Player))
	{
		PlayersInHealArea.Remove(Player);
		Player->Client_SetBonfireVignetteIntensity(0.0f);
	}
}

void APGInteractableGimmickBonfire::OnHealTick()
{
	if (!SanityHealEffectClass)
	{
		return;
	}

	for (auto It = PlayersInHealArea.CreateIterator(); It; ++It)
	{
		APGPlayerCharacter* Player = It->Get();
		if (!Player)
		{
			It.RemoveCurrent();
			continue;
		}

		Player->Server_ApplyGameplayEffectToSelf(SanityHealEffectClass);
	}
}

void APGInteractableGimmickBonfire::OnRep_IsLit()
{
	SetBonfireLit();
}

void APGInteractableGimmickBonfire::SetBonfireLit()
{
	if (bIsLit)
	{
		FireEffect->Activate(true);
		TargetLightIntensity = 900.0f;
		TargetEmissiveValue = 8.0f;

		SelfHighlightOff();

		PlayLocalSound(BoneFireStartSound, GetActorLocation());
	}
	else
	{
		FireEffect->Deactivate();
		TargetLightIntensity = 0.0f;
		TargetEmissiveValue = 0.0f;

		SelfHighlightOn();

		PlayLocalSound(BoneFireEndSound, GetActorLocation());
	}

	SetActorTickEnabled(true);
}