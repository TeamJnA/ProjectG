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

FText APGInteractableGimmickBonfire::GetInteractionText() const
{
	return bIsLit ? FText::GetEmpty() : FireText;
}

bool APGInteractableGimmickBonfire::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
{
	if (!bIsLit)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Match"))))
		{
			return true;
		}

		OutFailurePrompt.Icon = MatchIcon;
		OutFailurePrompt.IconSize = MatchIconSize;

		return false;
	}
	else
	{
		return false;
	}
}

void APGInteractableGimmickBonfire::InteractionFailed()
{
	PlayLocalSound(BoneFireFailedSound, GetActorLocation());
}

void APGInteractableGimmickBonfire::StartBonfire()
{
	if (!HasAuthority() || bIsLit)
	{
		return;
	}

	bIsLit = true;
	OnRep_IsLit();

	// 범위 내 플레이어 탐색 후 이펙트 적용
	TArray<AActor*> OverlappingActors;
	SanityHealAreaSphere->GetOverlappingActors(OverlappingActors, APGPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(Actor))
		{
			Player->Client_SetBonfireVignetteIntensity(VignetteIntensity);

			// ASC를 가져와서 이펙트를 적용하고, 플레이어 별 핸들을 Map에 저장
			if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddInstigator(this, this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SanityHealEffectClass, 1.0f, Context);

				if (SpecHandle.IsValid())
				{
					FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					ActiveHealEffectsMap.Add(Player, EffectHandle);
				}
			}
		}
	}

	// Bonfire Duration Timer
	GetWorld()->GetTimerManager().SetTimer(BoneFireDurationTimerHandle, this, &APGInteractableGimmickBonfire::StopBonfire, BonfireDuration, false);
}

void APGInteractableGimmickBonfire::StopBonfire()
{
	if (!HasAuthority() || !bIsLit)
	{
		return;
	}

	// GetWorld()->GetTimerManager().ClearTimer(SanityHealTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BoneFireDurationTimerHandle);

	// 추적 중인 플레이어들 Vignette off
	for (const auto& Pair : ActiveHealEffectsMap)
	{
		if (APGPlayerCharacter* Player = Pair.Key.Get())
		{
			Player->Client_SetBonfireVignetteIntensity(0.0f);

			// 활성화된 이펙트 제거
			if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
			{
				ASC->RemoveActiveGameplayEffect(Pair.Value);
			}
		}
	}
	ActiveHealEffectsMap.Empty();
		
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

	if (!ActiveHealEffectsMap.Contains(Player))
	{
		Player->Client_SetBonfireVignetteIntensity(VignetteIntensity);

		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddInstigator(this, this);
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SanityHealEffectClass, 1.0f, Context);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				ActiveHealEffectsMap.Add(Player, EffectHandle);
			}
		}
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

	if (FActiveGameplayEffectHandle* EffectHandlePtr = ActiveHealEffectsMap.Find(Player))
	{
		Player->Client_SetBonfireVignetteIntensity(0.0f);

		if (UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
		{
			ASC->RemoveActiveGameplayEffect(*EffectHandlePtr);
		}

		ActiveHealEffectsMap.Remove(Player);
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
		if (StaticMesh)
		{
			StaticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		}
		if (InteractCollisionBox)
		{
			InteractCollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		}

		PlayLocalSound(BoneFireStartSound, GetActorLocation());
	}
	else
	{
		FireEffect->Deactivate();
		TargetLightIntensity = 0.0f;
		TargetEmissiveValue = 0.0f;

		SelfHighlightOn();
		if (StaticMesh)
		{
			StaticMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
		if (InteractCollisionBox)
		{
			InteractCollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}

		PlayLocalSound(BoneFireEndSound, GetActorLocation());
	}

	SetActorTickEnabled(true);
}