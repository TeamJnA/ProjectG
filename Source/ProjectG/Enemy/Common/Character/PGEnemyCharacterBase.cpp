// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectG/AbilitySystem/PGAbilitySystemComponent.h"
#include "ProjectG/Enemy/Common/AbilitySystem/PGEnemyAttributeSet.h"










APGEnemyCharacterBase::APGEnemyCharacterBase()
{
	
	PrimaryActorTick.bCanEverTick = true;

	//ASC �ʱ�ȭ
	AbilitySystemComponent = CreateDefaultSubobject<UPGAbilitySystemComponent>("AbilitySystemComponent");
	//replication �ּ�ȭ (gameplay effect�� �������� ����)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	EnemyAttributeSet = CreateDefaultSubobject<UPGEnemyAttributeSet>("EnemyAttributeSet");
	
}




void APGEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	//ability component �ʱ�ȭ, owner actor, avatar actor. enemy ĳ���ʹ� �Ѵ� �ڱ��ڽ�


	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	//�⺻��ų �ο�
	GiveDefaultAbilities();

	InitDefaultAttributes();

	
}







UBehaviorTree* APGEnemyCharacterBase::GetBehaviorTree() const
{
	return Tree;
}


void APGEnemyCharacterBase::SetMovementSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}






//���ſ���




/*
APGPatrolPath* APGEnemyCharacterBase::GetPatrolPath() const
{
	return PatrolPath;
}*/
