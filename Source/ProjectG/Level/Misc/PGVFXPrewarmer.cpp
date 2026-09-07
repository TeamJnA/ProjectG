// Fill out your copyright notice in the Description page of Project Settings.

#include "Level/Misc/PGVFXPrewarmer.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Components/PrimitiveComponent.h"
#include "Game/PGAdvancedFriendsGameInstance.h"


APGVFXPrewarmer::APGVFXPrewarmer()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	SetActorHiddenInGame(true);
}

void APGVFXPrewarmer::BeginPlay()
{
	Super::BeginPlay();

	// 메인 메뉴에서 최초 1회만 프리워밍
	UPGAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UPGAdvancedFriendsGameInstance>();
	if (!GI || GI->LoadGameStateOnTravel() != EGameState::MainMenu || GI->HasPrewarmed())
	{
		UE_LOG(LogTemp, Log, TEXT("VFXPrewarmer: Already Prewarmed"));
		FinishPrewarm();
		return;
	}

	GI->MarkPrewarmed();
	GI->SetPrewarmInProgress(true);
	StartPrewarm();
}

void APGVFXPrewarmer::StartPrewarm()
{
	SetActorHiddenInGame(false);

	// Niagara
	TArray<UNiagaraComponent*> FXComps;
	GetComponents<UNiagaraComponent>(FXComps);
	for (UNiagaraComponent* FX : FXComps)
	{
		if (FX)
		{
			FX->Activate(true);
		}
	}

	// Texture, Mesh
	TArray<UMeshComponent*> MeshComps;
	GetComponents<UMeshComponent>(MeshComps);
	for (UMeshComponent* MeshComp : MeshComps)
	{
		if (MeshComp)
		{
			MeshComp->PrestreamTextures(5.0f, true);
		}
	}

	// Media
	if (MediaPlayerToPrewarm && MediaSourceToPrewarm)
	{
		MediaPlayerToPrewarm->SetLooping(true);
		MediaPlayerToPrewarm->OpenSource(MediaSourceToPrewarm);
		MediaPlayerToPrewarm->Play();
	}
}

void APGVFXPrewarmer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPrewarmDone)
	{
		return;
	}

	// 1) 렌더링 -> PSO 생성
	if (!bHideStarted)
	{
		if (++FrameCounter >= FramesToHold)
		{
			bHideStarted = true;

			TArray<UNiagaraComponent*> FXComps;
			GetComponents<UNiagaraComponent>(FXComps);
			for (UNiagaraComponent* FX : FXComps)
			{
				if (FX)
				{
					FX->Deactivate();
				}
			}

			if (MediaPlayerToPrewarm)
			{
				MediaPlayerToPrewarm->Pause();
			}

			SetActorHiddenInGame(true);
		}
		return;
	}

	// 2) 볼류메트릭 포그에 남은 잔광 제거 대기
	if (++FadeCounter >= FramesToFade)
	{
		FinishPrewarm();
	}
}

void APGVFXPrewarmer::FinishPrewarm()
{
	SetActorTickEnabled(false);
	bPrewarmDone = true;
	if (UPGAdvancedFriendsGameInstance* GI = GetWorld()->GetGameInstance<UPGAdvancedFriendsGameInstance>())
	{
		GI->SetPrewarmInProgress(false);
	}
	UE_LOG(LogTemp, Log, TEXT("VFXPrewarmer: done"));
}
