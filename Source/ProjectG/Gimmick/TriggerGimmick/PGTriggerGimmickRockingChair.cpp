// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/TriggerGimmick/PGTriggerGimmickRockingChair.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Character/PGPlayerCharacter.h"
#include "Sound/PGSoundManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"


APGTriggerGimmickRockingChair::APGTriggerGimmickRockingChair()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetNetUpdateFrequency(10.0f);

	// 베이스의 박스 트리거 사용 x
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->SetSphereRadius(1200.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetGenerateOverlapEvents(true);
}

FPhotoSubjectInfo APGTriggerGimmickRockingChair::GetPhotoSubjectInfo() const
{
	return FPhotoSubjectInfo(PhotoID::RockingChair, 10);
}

FVector APGTriggerGimmickRockingChair::GetPhotoTargetLocation() const
{
	return GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
}

void APGTriggerGimmickRockingChair::BeginPlay()
{
	Super::BeginPlay();

	BaseRotation = StaticMesh->GetRelativeRotation();

	if (HasAuthority())
	{
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &APGTriggerGimmickRockingChair::OnTriggerOverlap);
	}

	if (bIsRocking)
	{
		OnRep_IsRocking();
	}
}

void APGTriggerGimmickRockingChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGTriggerGimmickRockingChair, bIsRocking);
}

void APGTriggerGimmickRockingChair::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRocking || !StaticMesh)
	{
		return;
	}

	RockElapsed += DeltaTime;

	// 화면에 안 보이면 회전 갱신 x
	const bool bRecentlyRendered = StaticMesh->WasRecentlyRendered(0.5f);
	if (bRecentlyRendered)
	{
		const float Period = FMath::Max(RockPeriod, 0.1f);
		const float Phase = (RockElapsed / Period) * 2.0f * PI;
		const float Angle = FMath::Sin(Phase) * RockAngle;

		FRotator NewRotation = BaseRotation;
		NewRotation.Roll += Angle;
		StaticMesh->SetRelativeRotation(NewRotation);
	}

	// 소리는 벽 너머에서도 들려야 함 -> 계속 처리
	// PlayCreakSound에서 거리 기반 소리 재생 여부 판정
	const float Period = FMath::Max(RockPeriod, 0.1f);
	const int32 HalfCycle = FMath::FloorToInt(RockElapsed / (Period * 0.5f));
	if (HalfCycle != LastCreakHalfCycle)
	{
		LastCreakHalfCycle = HalfCycle;
		PlayCreakSound(CreakSoundFlipFlopState);
		CreakSoundFlipFlopState = !CreakSoundFlipFlopState;
	}
}

void APGTriggerGimmickRockingChair::OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || bIsRocking)
	{
		return;
	}

	if (!Cast<APGPlayerCharacter>(OtherActor))
	{
		return;
	}

	if (UKismetMathLibrary::RandomFloat() > ActivationChance)
	{
		return;
	}

	bIsRocking = true;
	OnRep_IsRocking();

	// bIsRocking은 다시 false로 돌아가지 않으므로 이후 복제 불필요
	SetNetDormancy(DORM_DormantAll);
}

void APGTriggerGimmickRockingChair::OnRep_IsRocking()
{
	RefreshPhotoRegistration();

	if (bIsRocking)
	{
		RockElapsed = 0.0f;
		LastCreakHalfCycle = -1;
		SetActorTickEnabled(true);
	}
}

void APGTriggerGimmickRockingChair::PlayCreakSound(bool FlipFlop)
{
	APGSoundManager* SM = SoundManager;
	if (!SM)
	{
		return;
	}

	// 로컬 거리 기반 재생 여부 판정
	if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (const APawn* LocalPawn = PC->GetPawn())
		{
			const uint8 Level = SM->GetSoundLevel(FlipFlop ? CreakSoundName : Creak2SoundName);
			const float AudibleRange = 200.0f * Level * Level;

			if (FVector::DistSquared(GetActorLocation(), LocalPawn->GetActorLocation())
				> FMath::Square(AudibleRange))
			{
				return;
			}
		}
	}

	SM->PlaySoundLocally(FlipFlop ? CreakSoundName : Creak2SoundName, GetActorLocation());
}
