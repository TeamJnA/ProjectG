// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_HandAction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/PGPlayerCharacter.h"

UGA_HandAction::UGA_HandAction()
{
	FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
	
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(HandActionActivateTag);

	ActivationBlockedTags.AddTag(HandActionActivateTag);
}

void UGA_HandAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Log, TEXT("Activate Handaction"));

	//Get HandAction Anim montage from PlayerCharacter and play montage
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	PG_CHECK_VALID_HANDACTION(AvatarActor);

	APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
	PG_CHECK_VALID_HANDACTION(PGPC);

	UAnimMontage* HandActionAnimMontage = PGPC->GetHandActionAnimMontages();
	PG_CHECK_VALID_HANDACTION(HandActionAnimMontage);

	//Prevent ability ended twice.
	bAbilityEnded = false;

	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("HandActionMontage"),
			HandActionAnimMontage,
			1.0f
		);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_HandAction::OnCompletedAnimMontage);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_HandAction::OnCompletedAnimMontage);
	MontageTask->ReadyForActivation();
}

void UGA_HandAction::OnCompletedAnimMontage()
{
	//Prevent ability ended twice.
	if (bAbilityEnded)
	{
		UE_LOG(LogTemp, Log, TEXT("Called ability ended twice."));
		return;
	}
	bAbilityEnded = true;

	UE_LOG(LogTemp, Log, TEXT("HandAction Completed"));

	//After finish hand action montage, equip and activate current index item on character.
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	PG_CHECK_VALID_HANDACTION(AvatarActor);

	APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
	PG_CHECK_VALID_HANDACTION(PGPC);

	PGPC->EquipCurrentInventoryItem();

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
