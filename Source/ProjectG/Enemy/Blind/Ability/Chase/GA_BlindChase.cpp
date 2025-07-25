// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/Chase/GA_BlindChase.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/CharacterMovementComponent.h"


UGA_BlindChase::UGA_BlindChase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	SetAssetTags(TagContainer);
	
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Chase")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));


	static ConstructorHelpers::FObjectFinder<UClass> ChaseGERef(TEXT("/Game/ProjectG/Enemy/Blind/Ability/Effect/GE_BlindChase.GE_BlindChase_C"));
	if (ChaseGERef.Object)
	{
		ChaseEffectClass = ChaseGERef.Object;
	}
}

void UGA_BlindChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ChaseEffectClass)
	{
		UE_LOG(LogTemp, Log, TEXT("DoChase"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChaseEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}




	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());


	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,                                 // Ability 객체
		TEXT("PlayRoarMontage"),                  // 태스크 이름 (디버깅용)
		OwnerPawn->RoarMontage,                            // 재생할 몽타주 에셋
		1.0f,                                // 재생 속도
		NAME_None,                           // 시작 섹션 이름 (필요시)
		true,                               // bStopWhenAbilityEnds (Ability 종료시 중단 여부)
		1.0f                               // 애니메이션 루트 모션 스케일
	);



	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_BlindChase::OnMontageCompleted);

	PlayMontageTask->ReadyForActivation();
		
}

void UGA_BlindChase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}

void UGA_BlindChase::OnMontageCompleted()
{
	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	OwnerPawn->SetHuntLevel(2);


	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}
