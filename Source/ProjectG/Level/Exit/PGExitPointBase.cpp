// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Exit/PGExitPointBase.h"
#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerState.h"
#include "Game/PGGameMode.h"
#include "Player/PGPlayerController.h"
#include "Interface/SoundManagerInterface.h"
#include "Sound/PGSoundManager.h"

// Sets default values
APGExitPointBase::APGExitPointBase()
{
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

void APGExitPointBase::OnEscapeStart(AActor* EscapeStartActor)
{
	if (APGPlayerCharacter* PlayerCharacter = Cast<APGPlayerCharacter>(EscapeStartActor))
	{
		// 컷신 -> 종료처리 -> 종료 카메라 뷰 변환 -> 스코어보드
		if (APGPlayerState* PS = PlayerCharacter->GetPlayerState<APGPlayerState>(); PS && !PS->HasFinishedGame())
		{
			if (APGGameMode* GM = GetWorld()->GetAuthGameMode<APGGameMode>())
			{
				GM->HandlePlayerEscaping(PlayerCharacter);
			}

			if (APGPlayerController* PC = Cast<APGPlayerController>(PlayerCharacter->GetController()))
			{
				PC->Client_StartEscapeSequence();
			}
		}
	}
}

