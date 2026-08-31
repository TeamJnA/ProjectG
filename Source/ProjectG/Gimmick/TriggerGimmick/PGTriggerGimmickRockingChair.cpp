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

	const float Period = FMath::Max(RockPeriod, 0.1f);
	const float Phase = (RockElapsed / Period) * 2.0f * PI;
	const float Angle = FMath::Sin(Phase) * RockAngle;

	FRotator NewRotation = BaseRotation;
	NewRotation.Roll += Angle;
	StaticMesh->SetRelativeRotation(NewRotation);

	// 양 끝점(사인 극값)에서 소리 발생
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
	if (APGSoundManager* SM = SoundManager)
	{
		if (FlipFlop)
		{
			SM->PlaySoundLocally(CreakSoundName, GetActorLocation());
		}
		else
		{
			SM->PlaySoundLocally(Creak2SoundName, GetActorLocation());
		}
	}
}
