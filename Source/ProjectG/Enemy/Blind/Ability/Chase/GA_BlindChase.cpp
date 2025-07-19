// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Blind/Ability/Chase/GA_BlindChase.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Character/Component/PGSoundManagerComponent.h"

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

	/*
	static ConstructorHelpers::FObjectFinder<UClass> ChaseGERef(TEXT("/Game/ProjectG/Enemy/Blind/Ability/Effect/GE_BlindChase.GE_BlindChase_C"));
	if (ChaseGERef.Object)
	{
		ChaseEffectClass = ChaseGERef.Object;
	}
	*/
}

void UGA_BlindChase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	/*
	if (ChaseEffectClass)
	{
		UE_LOG(LogTemp, Log, TEXT("DoChase"));
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(ChaseEffectClass);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	*/

	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find OwnerPawn in UGA_BlindChase"));
		return;
	}

	// Play roar sound before chasing.
	UPGSoundManagerComponent* SoundManager = GetAvatarActorFromActorInfo()->FindComponentByClass<UPGSoundManagerComponent>();
	if (SoundManager)
	{
		SoundManager->TriggerSoundForAllPlayers(TEXT("ENEMY_Roar"), OwnerPawn->GetActorLocation(), 5);
	}		
	else{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find SoundManager from AvatarActor in UGA_BlindChase"));
	}

	// Play a roar motion before beginning the chase.
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,                                 // Ability ��ü
		TEXT("PlayRoarMontage"),                  // �½�ũ �̸� (������)
		OwnerPawn->RoarMontage,                            // ����� ��Ÿ�� ����
		1.0f,                                // ��� �ӵ�
		NAME_None,                           // ���� ���� �̸� (�ʿ��)
		true,                               // bStopWhenAbilityEnds (Ability ����� �ߴ� ����)
		1.0f                               // �ִϸ��̼� ��Ʈ ��� ������
	);

	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_BlindChase::OnMontageCompleted);
	PlayMontageTask->ReadyForActivation();
		
}

void UGA_BlindChase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BlindChase::OnMontageCompleted()
{
	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	OwnerPawn->SetHuntLevel(2);


	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsChasing")));
}
