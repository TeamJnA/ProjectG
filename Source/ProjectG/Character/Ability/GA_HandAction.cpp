// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Ability/GA_HandAction.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/PGPlayerCharacter.h"
#include "AbilitySystemComponent.h"

UGA_HandAction::UGA_HandAction()
{
	FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));
	
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(HandActionActivateTag);

	ActivationBlockedTags.AddTag(HandActionActivateTag);

	//Add Event trigger
	FGameplayTag TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.HandAction"));
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = TriggerTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;

	AbilityTriggers.Add(TriggerData);

	bCameraOnEnded = false;

	// 서버가 먼저 handaction을 실행시키고 클라가 따라오도록
	// HandleGameplayEvent로 핸드액션 어빌을 실행시키는데, 이벤트를 보내는 경우는 local predict가 불가능
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UGA_HandAction::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("Activate Handaction"));

	//Get HandAction Anim montage from PlayerCharacter and play montage
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	PG_CHECK_VALID_HANDACTION(AvatarActor);

	APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
	PG_CHECK_VALID_HANDACTION(PGPC);

	check(TriggerEventData);
	EHandActionMontageType HandActionMontageType = static_cast<EHandActionMontageType>((int32)TriggerEventData->EventMagnitude);

	UE_LOG(LogTemp, Log, TEXT("Target Handaction : %d"),(int32)HandActionMontageType);

	// Select proper HandAction
	UAnimMontage* HandActionAnimMontage = nullptr;

	const int32 Index = static_cast<int32>(HandActionMontageType);

	if (HandActionAnimMontages.IsValidIndex(Index))
	{
		HandActionAnimMontage = HandActionAnimMontages[Index];
	}
	PG_CHECK_VALID_HANDACTION(HandActionAnimMontage);

	// HandAction이 CameraOn 일 경우 어빌리티 종료 시 핸드락을 걸어 준다.
	bCameraOnEnded = (HandActionMontageType == EHandActionMontageType::CameraOn);

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

	// CameraOn 몽타주의 경우, 끝나고 GE를 통한 Hand.Lock 부여.
	if (bCameraOnEnded)
	{
		PGPC->Server_SetHandLockByGameplayEffect(true);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}
