// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/PGProjectileItemBrick.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "PGLogChannels.h"
#include "Net/UnrealNetwork.h" 


APGProjectileItemBrick::APGProjectileItemBrick()
{
	/*
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	CollisionComponent->SetupAttachment(RootComponent);
	*/

	static ConstructorHelpers::FObjectFinder<UPGItemData> ItemDataRef(TEXT("/Game/ProjectG/Items/Consumable/DA_Consumable_Brick.DA_Consumable_Brick"));
	if (ItemDataRef.Object)
	{
		UPGItemData* ItemData = ItemDataRef.Object;

		ItemDataPath = ItemData;
		LoadedItemData = ItemData;
	}

	StaticMesh->OnComponentHit.AddDynamic(this, &APGProjectileItemBrick::OnHit);

	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;
	StaticMesh->BodyInstance.bLockZRotation = true;

	bAlreadyHit = false;
	bIsItem = false;

	LastBounceTime = 0.0f;
	PlaySoundCoolTime = 0.2f;

	MinBounceImpact = 100.0f;
}

void APGProjectileItemBrick::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	bAlreadyHit = true;

	// Brick hits once per actor.
	if (HitComponents.Contains(OtherComponent))
	{
		return;
	}
	HitComponents.Add(OtherComponent);

	// Play sound at hit location with cooltime.
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (CurrentTime - LastBounceTime >= PlaySoundCoolTime)
	{
		LastBounceTime = CurrentTime;

		// 일정 수준의 충돌 이상일 시 소리 재생
		if (NormalImpulse.Size() >= MinBounceImpact)
		{
			PlaySound(Hit.Location);
		}
	}

	// Check if the impact normal's Z component indicates a floor (upward-facing surface).
	// If true, disable physics and set the object as an item to pick and re-use.
	ECollisionChannel ObjectType = OtherComponent->GetCollisionObjectType();

	UE_LOG(LogItem, Log, TEXT("Brick hit with actor %s"), *OtherActor->GetName());
	
	if ((ObjectType == ECC_WorldDynamic || ObjectType == ECC_WorldStatic) && Hit.ImpactNormal.Z > 0.6)
	{
		bIsItem = true;
		ConvertIntoItem();
	}
	else
	{
		// 추가로 물체가 바닥에 닿지 않았으나 멈추는 경우를 계산하기 위한, 
		// 타이머를 통한 아이템 화 카운트 진행.
		GetWorld()->GetTimerManager().ClearTimer(ItemConvertTimer);

		GetWorld()->GetTimerManager().SetTimer(
			ItemConvertTimer,
			this,
			&APGProjectileItemBrick::ConvertIntoItemWithTimer,
			2.0f,
			false
		);
	}
}

void APGProjectileItemBrick::PlaySound_Implementation(const FVector& HitLocation)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			if (bAlreadyHit)
			{
				// TODO :: Apply ItemHitSound ITEM_Brick -> ITEM_Brick_Hit
				SoundManager->PlaySoundWithNoise(ItemHitSound, HitLocation, false);
			}
			else
			{
				SoundManager->PlaySoundWithNoise(ItemHitSound, HitLocation, false);
			}
		}
	}
}

void APGProjectileItemBrick::ConvertIntoItem()
{
	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->SetCollisionProfileName(TEXT("Item"));
}

void APGProjectileItemBrick::ConvertIntoItemWithTimer()
{
	bIsItem = true;
	ConvertIntoItem();
}

void APGProjectileItemBrick::OnRep_IsItem()
{
	ConvertIntoItem();
}

void APGProjectileItemBrick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGProjectileItemBrick, bIsItem);
}