// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/InteractableGimmick/PGInteractableGimmickPhone.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Sound/PGSoundManager.h"
#include "Utils/PGPhotoSubjectRegistry.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Perception/AISense_Hearing.h"


APGInteractableGimmickPhone::APGInteractableGimmickPhone()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ReceiverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReceiverMesh"));
	ReceiverMesh->SetupAttachment(RootComponent);
	ReceiverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollision"));
	InteractCollision->SetupAttachment(RootComponent);
	InteractCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	InteractCollision->SetGenerateOverlapEvents(false);

	EnterSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EnterSphere"));
	EnterSphere->SetupAttachment(RootComponent);
	EnterSphere->SetSphereRadius(400.0f);
	EnterSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EnterSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	EnterSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	EnterSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	EnterSphere->SetGenerateOverlapEvents(true);
	
	ExitSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ExitSphere"));
	ExitSphere->SetupAttachment(RootComponent);
	ExitSphere->SetSphereRadius(900.0f);
	ExitSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ExitSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ExitSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ExitSphere->SetGenerateOverlapEvents(true);

	RingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RingAudioComponent"));
	RingAudioComponent->SetupAttachment(RootComponent);
	RingAudioComponent->bAutoActivate = false;
}

FPhotoSubjectInfo APGInteractableGimmickPhone::GetPhotoSubjectInfo() const
{
	return FPhotoSubjectInfo(PhotoID::Phone, 10);
}

FVector APGInteractableGimmickPhone::GetPhotoTargetLocation() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 20.0f);
}

void APGInteractableGimmickPhone::RefreshPhotoRegistration()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UPGPhotoSubjectRegistry* Registry = World->GetSubsystem<UPGPhotoSubjectRegistry>();
	if (!Registry)
	{
		return;
	}

	if (IsPhotographable())
	{
		Registry->RegisterSubject(this);
	}
	else
	{
		Registry->UnregisterSubject(this);
	}
}

void APGInteractableGimmickPhone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		EnterSphere->OnComponentBeginOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnEnterSphereBeginOverlap);
		EnterSphere->OnComponentEndOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnEnterSphereEndOverlap);
		ExitSphere->OnComponentBeginOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnExitSphereBeginOverlap);
		ExitSphere->OnComponentEndOverlap.AddDynamic(this, &APGInteractableGimmickPhone::OnExitSphereEndOverlap);
	}

	if (ReceiverMesh)
	{
		MIDReceiver = ReceiverMesh->CreateDynamicMaterialInstance(0);
	}

	ApplyPhoneVisualState();
	RefreshPhotoRegistration();
}

void APGInteractableGimmickPhone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RingStartDelayHandle);
	GetWorldTimerManager().ClearTimer(RingTimerHandle);
	GetWorldTimerManager().ClearTimer(ShakeTimerHandle);

	if (UWorld* World = GetWorld())
	{
		if (UPGPhotoSubjectRegistry* Registry = World->GetSubsystem<UPGPhotoSubjectRegistry>())
		{
			Registry->UnregisterSubject(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void APGInteractableGimmickPhone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGInteractableGimmickPhone, PhoneState);
	DOREPLIFETIME(APGInteractableGimmickPhone, RingCount);
}

void APGInteractableGimmickPhone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHangUpPlaying || !ReceiverMesh)
	{
		SetActorTickEnabled(false);
		return;
	}

	HangUpElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(HangUpElapsed / FMath::Max(HangUpDuration, 0.01f), 0.0f, 1.0f);
	// 감속 보간
	const float Eased = 1.0f - FMath::Square(1.0f - Alpha);

	ReceiverMesh->SetRelativeLocation(FMath::Lerp(ReceiverStartLocation, HangUpRelativeLocation, Eased));
	ReceiverMesh->SetRelativeRotation(FMath::Lerp(ReceiverStartRotation.Quaternion(), HangUpRelativeRotation.Quaternion(), Eased));

	if (Alpha >= 1.0f)
	{
		ReceiverMesh->SetRelativeLocation(HangUpRelativeLocation);
		ReceiverMesh->SetRelativeRotation(HangUpRelativeRotation);
		bHangUpPlaying = false;
		SetActorTickEnabled(false);
	}
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

	StopRinging();
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
	return (PhoneState == EPGPhoneState::Ringing) ? OffText : FText::GetEmpty();
}

void APGInteractableGimmickPhone::OnEnterSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	// Blind 접근시 Idle로 전환
	if (Cast<APGBlindCharacter>(OtherActor))
	{
		bBlindInEnterSphere = true;
		StopRinging();
		return;
	}

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

void APGInteractableGimmickPhone::OnEnterSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Cast<APGBlindCharacter>(OtherActor))
	{
		bBlindInEnterSphere = false;
	}
}

void APGInteractableGimmickPhone::OnExitSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	APGPlayerState* PS = GetValidPlayerState(OtherActor);
	if (IsValidPhoneTarget(PS))
	{
		PlayersInRange.Add(PS);
	}
}

void APGInteractableGimmickPhone::OnExitSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

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

	// 적이 근처에 있으면 시작 x
	if (bBlindInEnterSphere)
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

	// 딜레이 후 플레이어가 없으면 취소 + Blind가 있으면 취소
	if (bBlindInEnterSphere || PlayersInRange.IsEmpty())
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

	/*
	if (APGSoundManager* SM = GetSoundManager())
	{
		SM->PlaySoundWithNoise(RingSoundName, GetActorLocation(), false, nullptr);
	}
	*/
	// Report Noise in server. 사운드 재생의 경우 Onrep을 통해 각자 재생
	const float NoiseLoudness = RingSoundNoiseLevel;
	const float NoiseMaxRange = RingSoundNoiseLevel * 200.0f; // 200.0f * NoiseLoudness

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		GetActorLocation(),
		NoiseLoudness,
		this,
		NoiseMaxRange,
		FName("PhoneRing") // 필요시 기존 사운드 FName으로 대체 가능
	);

	++RingCount;
	OnRep_RingCount();
}

void APGInteractableGimmickPhone::StopRinging()
{
	if (!HasAuthority() || PhoneState == EPGPhoneState::Disabled)
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

void APGInteractableGimmickPhone::OnRep_PhoneState()
{
	ApplyPhoneVisualState();
	RefreshPhotoRegistration();
	APGPlayerCharacter::NotifyLocalPlayerStareRefresh(this);

	if (PhoneState == EPGPhoneState::Disabled)
	{
		StartHangUpMotion();
		
		if (RingAudioComponent && RingAudioComponent->IsPlaying())
		{
			RingAudioComponent->Stop();
		}
	}
	else if (PhoneState == EPGPhoneState::Idle)
	{
		if (RingAudioComponent && RingAudioComponent->IsPlaying())
		{
			RingAudioComponent->Stop();
		}
	}	
}

void APGInteractableGimmickPhone::ApplyPhoneVisualState()
{
	const bool bRinging = (PhoneState == EPGPhoneState::Ringing);

	if (InteractCollision)
	{
		InteractCollision->SetCollisionResponseToChannel(ECC_Visibility, bRinging ? ECR_Block : ECR_Ignore);
	}

	if (StaticMesh)
	{
		StaticMesh->SetRenderCustomDepth(bRinging);
		if (bRinging)
		{
			StaticMesh->SetCustomDepthStencilValue(0);
		}
	}

	if (ReceiverMesh)
	{
		ReceiverMesh->SetRenderCustomDepth(bRinging);
		if (bRinging)
		{
			ReceiverMesh->SetCustomDepthStencilValue(0);
		}
	}

	if (!bRinging)
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

	if (RingAudioComponent)
	{
		RingAudioComponent->Play();
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

void APGInteractableGimmickPhone::StartHangUpMotion()
{
	if (!ReceiverMesh || bHangUpPlaying)
	{
		return;
	}

	ReceiverStartLocation = ReceiverMesh->GetRelativeLocation();
	ReceiverStartRotation = ReceiverMesh->GetRelativeRotation();
	HangUpElapsed = 0.0f;
	bHangUpPlaying = true;

	SetActorTickEnabled(true);
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
