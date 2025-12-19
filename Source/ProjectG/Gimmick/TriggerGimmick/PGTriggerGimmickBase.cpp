// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickBase.h"
#include "Components/BoxComponent.h"

#include "GameFramework/GameModeBase.h"
#include "Interface/GimmickTargetInterface.h"
#include "Sound/PGSoundManager.h"
#include "Interface/SoundManagerInterface.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
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

		if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
		{
			SoundManager = GameModeSoundManagerInterface->GetSoundManager();
			if (SoundManager)
			{
				UE_LOG(LogTemp, Log, TEXT("Init SoundManager Completely [%s]"), *GetNameSafe(this));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Init SoundManager Failed. Cannot find SoundManager in interface [%s]"), *GetNameSafe(this));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Init SoundManager Failed. Cannot find soundmanagerInterface [%s]"), *GetNameSafe(this));
		}
	}
}

void APGTriggerGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGTriggerGimmickBase, SoundManager);
}

void APGTriggerGimmickBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (UKismetMathLibrary::RandomFloat() > ActivationChance)
	{
		return;
	}

	APlayerState* TriggeringPS = Cast<ACharacter>(OtherActor) ? Cast<ACharacter>(OtherActor)->GetPlayerState() : nullptr;
	if (bIsOneShotEvent && TriggeringPS)
	{
		if (TriggeredPlayers.Contains(TriggeringPS))
		{
			UE_LOG(LogTemp, Log, TEXT("GimmickBase::OnTriggerOverlap: one-shot event"));
			return;
		}

		TriggeredPlayers.Add(TriggeringPS);
	}

	IGimmickTargetInterface* GimmickInterface = Cast<IGimmickTargetInterface>(OtherActor);
	if (GimmickInterface)
	{
		GimmickInterface->RequestApplyGimmickEffect(TriggerEffectClass);
	}

	Multicast_PlayLocalEffect(OtherActor, OtherComp);
}

void APGTriggerGimmickBase::Multicast_PlayLocalEffect_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
	if (ACharacter* Player = Cast<ACharacter>(OtherActor))
	{
		if (Player->IsLocallyControlled())
		{
			LocalEffect(OtherActor, OtherComp);
		}
	}
}
