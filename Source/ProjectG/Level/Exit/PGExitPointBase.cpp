// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitPointBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Game/PGGameMode.h"
#include "Player/PGPlayerController.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"
#include "Game/PGGameState.h"

// Sets default values
APGExitPointBase::APGExitPointBase()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	ExitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ExitCamera"));
	ExitCamera->SetupAttachment(Root);

	ExitPointType = EExitPointType::None;
}

void APGExitPointBase::BeginPlay()
{
	Super::BeginPlay();

	// Register Exit Camera both server and client
	RegisterExitCamera();
}

TSubclassOf<UGameplayAbility> APGExitPointBase::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGExitPointBase::HighlightOn() const
{
}

void APGExitPointBase::HighlightOff() const
{
}

FInteractionInfo APGExitPointBase::GetInteractionInfo() const
{
	return FInteractionInfo();
}

bool APGExitPointBase::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	return false;
}

bool APGExitPointBase::Unlock()
{
	return true;
}

void APGExitPointBase::PlaySound(const FName& SoundName, const FVector& SoundLocation)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			SoundManager->PlaySoundWithNoise(SoundName, SoundLocation, false);
		}
	}
}

void APGExitPointBase::PlaySoundPlayers(const FName& SoundName, const FVector& SoundLocation)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			SoundManager->PlaySoundForAllPlayers(SoundName, SoundLocation);
		}
	}
}

void APGExitPointBase::OnEscapeStart(AActor* EscapeStartActor, EExitPointType InExitPoint)
{
	if (APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(EscapeStartActor))
	{
		// 컷신 -> 종료처리 -> 종료 카메라 뷰 변환 -> 스코어보드
		if (APGPlayerState* PS = PlayerCharacter->GetPlayerState<APGPlayerState>(); PS && !PS->HasFinishedGame())
		{
			if (APGGameMode* GM = GetWorld()->GetAuthGameMode<APGGameMode>())
			{
				GM->HandlePlayerEscaping(PlayerCharacter, InExitPoint);
			}

			if (APGPlayerController* PC = Cast<APGPlayerController>(PlayerCharacter->GetController()))
			{
				PC->Client_StartEscapeSequence(InExitPoint);
			}
		}
	}
}

// Register Exit Camera both server and client
void APGExitPointBase::RegisterExitCamera()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (APGGameState* GS = World->GetGameState<APGGameState>())
	{
		GS->RegisterExitCamera(ExitPointType, this);

		UE_LOG(LogPGExitPoint, Log, TEXT("%s Registered to GameState successfully."), *GetNameSafe(this));
	}
	else
	{
		// 최대 10번(약 5초)까지만 재시도하도록 방어 로직 추가
		if (RegistrationRetries < 10)
		{
			RegistrationRetries++;

			// NextTick 대신 0.5초 간격으로 본인 함수 다시 호출
			FTimerHandle TempHandle;
			World->GetTimerManager().SetTimer(TempHandle, this, &APGExitPointBase::RegisterExitCamera, 0.5f, false);

			UE_LOG(LogPGExitPoint, Warning, TEXT("GameState not found. Retrying... (%d/10)"), RegistrationRetries);
		}
		else
		{
			UE_LOG(LogPGExitPoint, Error, TEXT("Failed to register %s: GameState is null after retries."), *GetNameSafe(this));
		}
	}
}

