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

//gameplay Attribute ���� ����Ǳ� ���� ȣ���.
void UPGEnemyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMovementSpeed());
	}
}

//gameplay effect�� ����� �Ŀ� ȣ��� . �ش� �Ӽ��� ��� ó������ ����.
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

	/* �䷱ ������ ���ٰ���.

	UAbilitySystemComponent* ASC = Data.EffectSpec.GetContext().GetInstigatorAbilitySystemComponent();
	AActor* ASCOwner = ASC->AbilityActorInfo->OwnerActor.Get();
	ASCOwner->GetActorLocation();*/
}

void UPGEnemyAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed) const
{
	//������ �Ӽ� ó���ϴ� ��ũ��.
	//���������� 
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
