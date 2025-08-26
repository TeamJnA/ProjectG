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

DEFINE_LOG_CATEGORY(LogEnemyCharacter);

APGEnemyCharacterBase::APGEnemyCharacterBase()
{
	
	PrimaryActorTick.bCanEverTick = true;

	//ASC 초기화
	AbilitySystemComponent = CreateDefaultSubobject<UPGAbilitySystemComponent>("AbilitySystemComponent");
	//replication 최소화 (gameplay effect는 복제되지 않음)
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	EnemyAttributeSet = CreateDefaultSubobject<UPGEnemyAttributeSet>("EnemyAttributeSet");

	
	TouchCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TouchCollider"));
	TouchCollider->SetupAttachment(RootComponent);
	TouchCollider->SetBoxExtent(FVector(50.f)); // 기본 크기 (BP에서 조정 가능)
	TouchCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TouchCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	TouchCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	

	SoundManagerComponent = CreateDefaultSubobject<UPGSoundManagerComponent>(TEXT("SoundManagerComponent"));
}

void APGEnemyCharacterBase::NotifyAttackEnded()
{
	if (HasAuthority())
	{
		UE_LOG(LogEnemyCharacter, Log, TEXT("%s Attack is Finished."), *GetNameSafe(this));
		if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(CachedAttackedTarget))
		{
			AttackableInterface->OnAttackFinished();
		}

		CachedAttackedTarget = nullptr;
	}
}

void APGEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	//ability component 초기화, owner actor, avatar actor. enemy 캐릭터는 둘다 자기자신


	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	//기본스킬 부여
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
	// Overlap only perform on server. ( Sync issue )
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
		return;

	// If the other actor can attackable.
	if (IAttackableTarget* AttackableInterface= Cast<IAttackableTarget>(OtherActor))
	{
		// Check if the target is valid. ( check already dead or broken ).
		// And to prevent duplicate, check otheractor is not CachedAttackedTarget.
		if (AttackableInterface->IsValidAttackableTarget() && OtherActor != CachedAttackedTarget)
		{
			// Caching actor to notify the target that the attack has finished.
			CachedAttackedTarget = OtherActor;

			UE_LOG(LogEnemyCharacter, Log, TEXT("[%s] find attackable target."), *GetNameSafe(this));
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
