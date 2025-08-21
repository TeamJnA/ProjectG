// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectG/Enemy/Common/Character/PGEnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "ProjectG/AbilitySystem/PGAbilitySystemComponent.h"
#include "ProjectG/Enemy/Common/AbilitySystem/PGEnemyAttributeSet.h"

#include "Perception/AISense_Touch.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

#include "Character/Component/PGSoundManagerComponent.h"

#include "Interface/AttackableTarget.h"


APGEnemyCharacterBase::APGEnemyCharacterBase()
{
	
	PrimaryActorTick.bCanEverTick = true;

	//ASC �ʱ�ȭ
	AbilitySystemComponent = CreateDefaultSubobject<UPGAbilitySystemComponent>("AbilitySystemComponent");
	//replication �ּ�ȭ (gameplay effect�� �������� ����)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	EnemyAttributeSet = CreateDefaultSubobject<UPGEnemyAttributeSet>("EnemyAttributeSet");

	
	TouchCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TouchCollider"));
	TouchCollider->SetupAttachment(RootComponent);
	TouchCollider->SetBoxExtent(FVector(50.f)); // �⺻ ũ�� (BP���� ���� ����)
	TouchCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TouchCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TouchCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	

	SoundManagerComponent = CreateDefaultSubobject<UPGSoundManagerComponent>(TEXT("SoundManagerComponent"));
}




void APGEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	//ability component �ʱ�ȭ, owner actor, avatar actor. enemy ĳ���ʹ� �Ѵ� �ڱ��ڽ�


	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	//�⺻��ų �ο�
	InitDefaultAttributes();
	GiveAndActivatePassiveEffects();
	GiveDefaultAbilities();

	if (TouchCollider)
	{
		TouchCollider->OnComponentBeginOverlap.AddDynamic(this, &APGEnemyCharacterBase::OnTouchColliderOverlapBegin);
	}
}

void APGEnemyCharacterBase::OnTouchColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	// If the other actor can attackable.
	if (IAttackableTarget* AttackableInterface= Cast<IAttackableTarget>(OtherActor))
	{
		// Check if the target is valid. ( check already dead or broken ).
		if (AttackableInterface->IsValidAttackableTarget())
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] find attackable target."), *GetNameSafe(this));
			UAISense_Touch::ReportTouchEvent(GetWorld(), this, OtherActor, OtherActor->GetActorLocation());
			AttackableInterface->OnAttacked(GetCapsuleTopWorldLocation());
		}
	}
}

FVector APGEnemyCharacterBase::GetCapsuleTopWorldLocation() const
{
	const UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(GetRootComponent());

	if (!Capsule)
	{
		return GetActorLocation();
	}

	const FVector Center = Capsule->GetComponentLocation();
	const FVector Up = Capsule->GetUpVector();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	return (Center + Up * HalfHeight);
}

UBehaviorTree* APGEnemyCharacterBase::GetBehaviorTree() const
{
	return Tree;
}


void APGEnemyCharacterBase::SetMovementSpeed(float speed)
{
	GetCharacterMovement()->MaxWalkSpeed = speed;
}

void APGEnemyCharacterBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
}
