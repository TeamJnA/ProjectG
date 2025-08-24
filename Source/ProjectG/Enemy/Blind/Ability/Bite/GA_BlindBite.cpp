// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Blind/Ability/Bite/GA_BlindBite.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Blind/Character/PGBlindCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Enemy/Blind/AI/Controllers/PGBlindAIController.h"

UGA_BlindBite::UGA_BlindBite()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack.Bite")));
	SetAssetTags(TagContainer);

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack.Bite")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack.Bite")));

	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior")));
}

void UGA_BlindBite::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find AvatarActor from UGA_BlindBite"));
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find AIController in UGA_BlindBite"));
		return;
	}

	UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent();
	if (!Blackboard)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot find Blackboard in UGA_BlindBite"));
		return;
	}

	Blackboard->SetValueAsBool(FName("DetectedPlayer"), true); //Behavior tree�� Detected Player �� ����.


	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,                                 // Ability ��ü
		TEXT("PlayBiteMontage"),                  // �½�ũ �̸� (������)
		OwnerPawn->BiteMontage,                            // ����� ��Ÿ�� ����
		1.0f,                                // ��� �ӵ�
		NAME_None,                           // ���� ���� �̸� (�ʿ��)
		true,                               // bStopWhenAbilityEnds (Ability ����� �ߴ� ����)
		1.0f                               // �ִϸ��̼� ��Ʈ ��� ������
	);

	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_BlindBite::OnMontageCompleted);


	GetAbilitySystemComponentFromActorInfo()->
		AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking.IsBiting")));

	PlayMontageTask->ReadyForActivation();
}

void UGA_BlindBite::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetAbilitySystemComponentFromActorInfo()->
		RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("AI.State.IsAttacking.IsBiting")));

	if (APGBlindCharacter* OwnerPawn = Cast<APGBlindCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
		{
			if(UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
				Blackboard->SetValueAsBool(FName("DetectedPlayer"), false);//Behavior tree�� Detected Player �� ����.

			if (APGBlindAIController* AIC = Cast<APGBlindAIController>(AIController))
			{
				AIC->ResetHuntLevel();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BlindBite::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
}