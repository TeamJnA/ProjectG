// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectG/AbilitySystem/PGAbilitySystemComponent.h"
#include "ProjectG/Enemy/Common/AbilitySystem/PGEnemyAttributeSet.h"










APGEnemyCharacterBase::APGEnemyCharacterBase()
{
	
	PrimaryActorTick.bCanEverTick = true;

	//ASC 초기화
	AbilitySystemComponent = CreateDefaultSubobject<UPGAbilitySystemComponent>("AbilitySystemComponent");
	//replication 최소화 (gameplay effect는 복제되지 않음)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	EnemyAttributeSet = CreateDefaultSubobject<UPGEnemyAttributeSet>("EnemyAttributeSet");
	
}




void APGEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	//ability component 초기화, owner actor, avatar actor. enemy 캐릭터는 둘다 자기자신


	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	//기본스킬 부여
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






//제거예정




/*
APGPatrolPath* APGEnemyCharacterBase::GetPatrolPath() const
{
	return PatrolPath;
}*/
