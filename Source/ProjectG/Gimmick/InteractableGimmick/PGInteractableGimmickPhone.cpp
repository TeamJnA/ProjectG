// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickPhone.h"
#include "Components/SphereComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Sound/PGSoundManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"


APGInteractableGimmickPhone::APGInteractableGimmickPhone()
{
	PrimaryActorTick.bCanEverTick = false;

	ReceiverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReceiverMesh"));
	ReceiverMesh->SetupAttachment(StaticMesh);
	ReceiverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EnterSphere"));
	EnterSphere->SetupAttachment(RootComponent);
	EnterSphere->SetSphereRadius(400.0f);
	EnterSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnterSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EnterSphere->SetGenerateOverlapEvents(true);
	
	ExitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExitSphere"));
	ExitSphere->SetupAttachment(RootComponent);
	ExitSphere->SetSphereRadius(900.0f);
	ExitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExitSphere->SetGenerateOverlapEvents(true);
}

void APGInteractableGimmickPhone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		EnterSphere->OnComponentBeginOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnEnterSphereBeginOverlap);
		ExitSphere->OnComponentBeginOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnExitSphereBeginOverlap);
		ExitSphere->OnComponentEndOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnExitSphereEndOverlap);
	}

	if (ReceiverMesh)
	{
		MIDReceiver = ReceiverMesh->CreateDynamicMaterialInstance(0);
	}

	ApplyPhoneVisualState();
}

void APGInteractableGimmickPhone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RingStartDelayHandle);
	GetWorldTimerManager().ClearTimer(RingTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APGInteractableGimmickPhone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGInteractableGimmickPhone, PhoneState);
	DOREPLIFETIME(APGInteractableGimmickPhone, RingCount);
}

void APGInteractableGimmickPhone::GimmickInteract(AActor* Investigator)
{
	if (!HasAuthority() || PhoneState != EPGPhoneState::Ringing)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RingTimerHandle);
	SetPhoneState(EPGPhoneState::Disabled);

	if (APGSoundManager* SM = GetSoundManager())
	{
		SM->PlaySoundForAllPlayers(HangUpSoundName, GetActorLocation());
	}
}

void APGInteractableGimmickPhone::HighlightOn() const
{
}

void APGInteractableGimmickPhone::HighlightOff() const
{
}

FInteractionInfo APGInteractableGimmickPhone::GetInteractionInfo() const
{
	return FInteractionInfo(EInteractionType::Hold, InteractHoldDuration);
}

FText APGInteractableGimmickPhone::GetInteractionText() const
{
	return (PhoneState == EPGPhoneState::Ringing) ? AnswerText : FText::GetEmpty();
}

void APGInteractableGimmickPhone::OnEnterSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (PhoneState != EPGPhoneState::Idle)
	{
		return;
	}

	APGPlayerState* PS = GetValidPlayerState(OtherActor);
	if (!IsValidPhoneTarget(PS) || RolledPlayers.Contains(PS))
	{
		return;
	}

	// 작동하든 안하든 판정 완료 처리
	RolledPlayers.Add(PS);

	if (UKismetMathLibrary::RandomFloat() > ActivationChance)
	{
		return;
	}

	TryStartRinging();
}

void APGInteractableGimmickPhone::OnExitSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APGPlayerState* PS = GetValidPlayerState(OtherActor);
	if (IsValidPhoneTarget(PS))
	{
		PlayersInRange.Add(PS);
	}
}

void APGInteractableGimmickPhone::OnExitSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APGPlayerState* PS = GetValidPlayerState(OtherActor);
	if (!PS)
	{
		return;
	}

	PlayersInRange.Remove(PS);
	RolledPlayers.Remove(PS);

	// 아무도 없으면 울림 중단 (다시 울릴 수 있는 상태)
	if (PhoneState == EPGPhoneState::Ringing && PlayersInRange.IsEmpty())
	{
		StopRinging();
	}
}

void APGInteractableGimmickPhone::TryStartRinging()
{
	if (!HasAuthority() || PhoneState != EPGPhoneState::Idle)
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(RingStartDelayHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(RingStartDelayHandle, this, &APGInteractableGimmickPhone::StartRinging, RingStartDelay, false);
}

void APGInteractableGimmickPhone::StartRinging()
{
	if (!HasAuthority() || PhoneState != EPGPhoneState::Idle)
	{
		return;
	}

	// 딜레이 후 플레이어가 없으면 취소
	if (PlayersInRange.IsEmpty())
	{
		return;
	}

	SetPhoneState(EPGPhoneState::Ringing);

	PlayRingSound();
	GetWorldTimerManager().SetTimer(RingTimerHandle, this, &APGInteractableGimmickPhone::PlayRingSound, RingInterval, true);
}

void APGInteractableGimmickPhone::PlayRingSound()
{
	// 무효 플레이어 정리(사망)
	for (auto It = PlayersInRange.CreateIterator(); It; ++It)
	{
		if (!IsValidPhoneTarget(It->Get()))
		{
			It.RemoveCurrent();
		}
	}

	if (PlayersInRange.IsEmpty())
	{
		StopRinging();
		return;
	}

	if (APGSoundManager* SM = GetSoundManager())
	{
		SM->PlaySoundWithNoise(RingSoundName, GetActorLocation(), false, nullptr);
	}

	++RingCount;
	OnRep_RingCount();
}

void APGInteractableGimmickPhone::StopRinging()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RingStartDelayHandle);
	GetWorldTimerManager().ClearTimer(RingTimerHandle);
	SetPhoneState(EPGPhoneState::Idle);
}

void APGInteractableGimmickPhone::SetPhoneState(EPGPhoneState NewState)
{
	if (PhoneState == NewState)
	{
		return;
	}

	PhoneState = NewState;
	OnRep_PhoneState();
}

bool APGInteractableGimmickPhone::IsValidPhoneTarget(const APGPlayerState* PS) const
{
	return PS && !PS->IsDead() && PS->IsInGame();
}

APGPlayerState* APGInteractableGimmickPhone::GetValidPlayerState(AActor* OtherActor) const
{
	APGPlayerCharacter* Player = Cast<APGPlayerCharacter>(OtherActor);
	return Player ? Player->GetPlayerState<APGPlayerState>() : nullptr;
}

void APGInteractableGimmickPhone::OnRep_PhoneState()
{
	ApplyPhoneVisualState();
	APGPlayerCharacter::NotifyLocalPlayerStareRefresh(this);
}

void APGInteractableGimmickPhone::ApplyPhoneVisualState()
{
	if (!StaticMesh)
	{
		return;
	}

	const bool bRinging = (PhoneState == EPGPhoneState::Ringing);
	StaticMesh->SetCollisionResponseToChannel(ECC_Visibility, bRinging ? ECR_Block : ECR_Ignore);
	StaticMesh->SetRenderCustomDepth(bRinging);
	if (bRinging)
	{
		StaticMesh->SetCustomDepthStencilValue(0);
		ReceiverMesh->SetCustomDepthStencilValue(0);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(ShakeTimerHandle);
		StopShake();
	}
}

void APGInteractableGimmickPhone::OnRep_RingCount()
{
	if (!MIDReceiver)
	{
		return;
	}

	MIDReceiver->SetScalarParameterValue(ShakeParameterName, ShakePower);
	GetWorldTimerManager().SetTimer(ShakeTimerHandle, this, &APGInteractableGimmickPhone::StopShake, ShakeDuration, false);
}

void APGInteractableGimmickPhone::StopShake()
{
	if (MIDReceiver)
	{
		MIDReceiver->SetScalarParameterValue(ShakeParameterName, 0.0f);
	}
}
