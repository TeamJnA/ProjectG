// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Components/BoxComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APGTriggerGimmickBase::APGTriggerGimmickBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(RootComponent);
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void APGTriggerGimmickBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &APGTriggerGimmickBase::OnTriggerOverlap);
	}
}

void APGTriggerGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGTriggerGimmickBase, bHasBeenTriggered);
}

void APGTriggerGimmickBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || !HasAuthority())
	{
		return;
	}

	if (bIsOneShotEvent && bHasBeenTriggered)
	{
		return;
	}

	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		if (UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent())
		{
			TryActivateEvent(TargetASC);
		}
	}
}

void APGTriggerGimmickBase::TryActivateEvent(UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC || !TriggerEffectClass)
	{
		return;
	}

	if (UKismetMathLibrary::RandomFloat() <= ActivationChance)
	{
		UE_LOG(LogTemp, Log, TEXT("TriggerGimmickBase::TryActivateEvent: Activated! Applying effect."));
		FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(TriggerEffectClass, 1.0f, EffectContext);
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		if (bIsOneShotEvent)
		{
			bHasBeenTriggered = true;
			TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

