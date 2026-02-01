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
#include "Level/Misc/PGDoor1.h"
#include "Type/CharacterTypes.h"

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
	TouchCollider->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	
	bDoorBreakOpen = false;

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

FGenericTeamId APGEnemyCharacterBase::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)EGameTeam::AI);
}

ETeamAttitude::Type APGEnemyCharacterBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other))
	{
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

		if (OtherTeamId == (uint8)EGameTeam::Player)
		{
			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Neutral;
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
	{
		return;
	}

	// If the other actor can attackable.
	// TODO TARRAY 들어오면 담고 나가면 빼. 죽일때 빼. 공격이 끝났을때 -> 껐다키는거랑 똑같은데?  array 가 비었는지 확인하고 안비면 다시 루프돎. 
	// 반복~ 플레이어가 컬라이더에서 나왔어. endoverlap 인식이 안 됨. 이럴경우엔 어떡하냐.. -> 공격하기전에 대상이 유효한지(거리를 다시 확인한다)
	// 현재 주변 체크 하는 기능 -> 이게 가능하면 이게 훨씬 낫다. 
	// 유지보수에안좋아. // 분류가안됨. 
	// 
	if (IAttackableTarget* AttackableInterface= Cast<IAttackableTarget>(OtherActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Player was detected by %s"), *GetNameSafe(this));
		OnPlayerOverlapped(OtherActor);
		return;
	}

	// Door detected
	if (APGDoor1* OverlappedDoor = Cast<APGDoor1>(OtherActor))
	{
		UE_LOG(LogTemp, Log, TEXT("Door was detected by %s"), *GetNameSafe(this));
		OnDoorOverlapped(OtherActor);
		return;
	}
}

void APGEnemyCharacterBase::OnPlayerOverlapped(AActor* OverlapPlayer)
{
	if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(OverlapPlayer))
	{
		// Check if the target is valid. ( check already dead or broken ).
		// And to prevent duplicate, check otheractor is not CachedAttackedTarget.
		if (AttackableInterface->IsValidAttackableTarget() && OverlapPlayer != CachedAttackedTarget)
		{
			// Caching actor to notify the target that the attack has finished.
			CachedAttackedTarget = OverlapPlayer;

			UE_LOG(LogEnemyCharacter, Log, TEXT("[%s] find attackable target."), *GetNameSafe(this));
			UAISense_Touch::ReportTouchEvent(GetWorld(), this, OverlapPlayer, OverlapPlayer->GetActorLocation());

			AttackableInterface->OnAttacked(GetCapsuleTopWorldLocation());
		}
	}
}

void APGEnemyCharacterBase::OnDoorOverlapped(AActor* OverlapDoor)
{
	if (APGDoor1* OverlappedDoor = Cast<APGDoor1>(OverlapDoor))
	{
		if (bDoorBreakOpen == true)
		{
			// TODO : Make door break hear
			OverlappedDoor->TEST_OpenDoorByAI(this);
		}
		else
		{
			OverlappedDoor->TEST_OpenDoorByAI(this);
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

// Open(Break) all doors around character.
// When hunt level become 1 or 2 from 0, this function called to break doors.
void APGEnemyCharacterBase::ForceOpenDoorsAroundCharacter()
{
	TArray<AActor*> OverlappedActors;
	TouchCollider->GetOverlappingActors(OverlappedActors);
	for (AActor* OverlappedActor : OverlappedActors)
	{
		APGDoor1* OverlappedDoor = Cast<APGDoor1>(OverlappedActor);
		if (OverlappedDoor)
		{
			UE_LOG(LogTemp, Log, TEXT("Door around EnemyCharacterBase was detected"));
			OverlappedDoor->TEST_OpenDoorByAI(this);
		}
	}
}