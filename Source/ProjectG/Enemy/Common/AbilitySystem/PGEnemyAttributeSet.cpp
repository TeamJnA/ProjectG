// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Common/AbilitySystem/PGEnemyAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"

UPGEnemyAttributeSet::UPGEnemyAttributeSet()
{
}

void UPGEnemyAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/// With this macro, OnRep_Stamina() is called on the client whenever the stamina value changes.
	/// COND_None : Replicated to all clients. (ex. health bar should be replicated to all clients so everyone can see health)
	/// COND_OwnerOnly : Replicated to only actors owner.
	
	DOREPLIFETIME_CONDITION_NOTIFY(UPGEnemyAttributeSet, MovementSpeed, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPGEnemyAttributeSet, MaxMovementSpeed, COND_OwnerOnly, REPNOTIFY_Always);
}

//gameplay Attribute 값이 변경되기 전에 호출됨.
void UPGEnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMovementSpeed());
	}
}

//gameplay effect가 실행된 후에 호출됨 . 해당 속성을 어떻게 처리할지 결정.
void UPGEnemyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		SetMovementSpeed(FMath::Clamp(GetMovementSpeed(), 0.f, GetMaxMovementSpeed()));

		AActor* Owner = GetOwningActor();
		if (APGEnemyCharacterBase* OwnerPawn = Cast<APGEnemyCharacterBase>(Owner))
		{
			OwnerPawn->SetMovementSpeed(MovementSpeed.GetCurrentValue());
		}
	}

	/* 요런 식으로 접근가능.

	UAbilitySystemComponent* ASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
	AActor* ASCOwner = ASC->AbilityActorInfo->OwnerActor.Get();
	ASCOwner->GetActorLocation();*/
}

void UPGEnemyAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed) const
{
	//복제된 속성 처리하는 매크로.
	//내부적으로 
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGEnemyAttributeSet, MovementSpeed, OldMovementSpeed);

	AActor* Owner = GetOwningActor();
	if (APGEnemyCharacterBase* OwnerPawn = Cast<APGEnemyCharacterBase>(Owner))
	{
		OwnerPawn->SetMovementSpeed(MovementSpeed.GetCurrentValue());
	}
}

void UPGEnemyAttributeSet::OnRep_MaxMovementSpeed(const FGameplayAttributeData& OldMaxMovementSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPGEnemyAttributeSet, MaxMovementSpeed, OldMaxMovementSpeed);
}
