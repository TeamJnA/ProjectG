// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Charger/Character/PGChargerCharacter.h"
#include "Enemy/Charger/Animation/AnimInstance/PGChargerAnimInstance.h"
#include "Enemy/Charger/AI/Controller/PGChargerAIController.h"
#include "Enemy/Charger/Ability/Kill/GA_ChargerKill.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interface/AttackableTarget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Character/Component/PGSoundManagerComponent.h"

APGChargerCharacter::APGChargerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
}

void APGChargerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetMovementSpeed(PatrolSpeed);

	// HeadLookAtLocation = GetActorLocation() + GetActorForwardVector() * 1000.0f + FVector(0.0f, 0.0f, 150.0f);
}

void APGChargerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGChargerCharacter, HeadYaw);
	DOREPLIFETIME(APGChargerCharacter, HeadLookAtLocation);
	DOREPLIFETIME(APGChargerCharacter, CurrentState);
}

void APGChargerCharacter::GetActorEyesViewPoint(FVector& out_Location, FRotator& out_Rotation) const
{
	const FTransform HeadTransform = GetMesh()->GetSocketTransform(TEXT("head"), RTS_World);

	//out_Location = HeadTransform.GetLocation();
	//out_Rotation = HeadTransform.GetUnitAxis(EAxis::Y).Rotation();

	out_Location = GetActorLocation();

	FRotator RawHeadRot = HeadTransform.GetUnitAxis(EAxis::Y).Rotation();
	RawHeadRot.Roll = 0.0f;
	RawHeadRot.Pitch = 0.0f;
	out_Rotation = RawHeadRot;
}

void APGChargerCharacter::SetHeadLookAtTarget(const FVector& NewTargetLocation)
{
	if (HasAuthority())
	{
		HeadLookAtLocation = NewTargetLocation;
	}
}

FVector APGChargerCharacter::GetHeadLookAtTarget() const
{
	if (CurrentState == E_PGChargerState::Killing)
	{
		if (CachedAttackedTarget)
		{
			return CachedAttackedTarget->GetActorLocation();
		}

		return GetActorLocation() + GetActorForwardVector() * 500.0f;
	}

	return HeadLookAtLocation;
}

void APGChargerCharacter::SetMovementSpeed(float NewSpeed)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!MovementSpeedEffectClass || !ASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(this, this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(MovementSpeedEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid())
	{
		FGameplayTag SpeedTag = FGameplayTag::RequestGameplayTag(FName("AI.Data.MovementSpeed"));
		SpecHandle.Data->SetSetByCallerMagnitude(SpeedTag, NewSpeed);

		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void APGChargerCharacter::RotateHeadYaw()
{
	if (HasAuthority())
	{
		if (HeadYaw >= 360.0f)
		{
			if (UPGChargerAnimInstance* AnimInst = Cast<UPGChargerAnimInstance>(GetMesh()->GetAnimInstance()))
			{
				AnimInst->ApplyRotationOffset(360.0f); 
			}
			HeadYaw -= 360.0f;
		}
		SoundManagerComponent->TriggerSoundForAllPlayers(HeadRotate180Name, GetActorLocation());

		HeadYaw += 180.0f;
	}
}

void APGChargerCharacter::SetCurrentState(E_PGChargerState NewState)
{
	if (HasAuthority())
	{
		if (CurrentState == NewState) 
		{
			return;
		}

		CurrentState = NewState;
		if (CurrentState == E_PGChargerState::Adjusting)
		{
			SetMovementSpeed(AdjustSpeed);
		}
		else if (CurrentState == E_PGChargerState::Attacking)
		{
			SetMovementSpeed(ChargeSpeed);
		}
		else if(CurrentState == E_PGChargerState::Staring)
		{
			SoundManagerComponent->TriggerSoundForAllPlayers(HeadRotateStareName, GetActorLocation());
			SetMovementSpeed(PatrolSpeed);
		}
		else
		{
			SetMovementSpeed(PatrolSpeed);
		}
	}
}

void APGChargerCharacter::OnRep_HeadYaw(float OldValue)
{
	if (HeadYaw < OldValue)
	{
		if (UPGChargerAnimInstance* AnimInst = Cast<UPGChargerAnimInstance>(GetMesh()->GetAnimInstance()))
		{
			AnimInst->ApplyRotationOffset(360.0f);
		}
	}
}

void APGChargerCharacter::OnTouchColliderOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (IAttackableTarget* AttackableInterface = Cast<IAttackableTarget>(OtherActor))
	{
		if (AttackableInterface->IsValidAttackableTarget() && OtherActor != CachedAttackedTarget)
		{
			CachedAttackedTarget = OtherActor;

			if (APGChargerAIController* AIC = Cast<APGChargerAIController>(GetController()))
			{
				AIC->GetBlackboardComponent()->SetValueAsEnum(APGChargerAIController::BlackboardKey_AIState, (uint8)E_PGChargerState::Killing);
				SetCurrentState(E_PGChargerState::Killing);
			}

			FVector KillOffset = GetCapsuleTopWorldLocation();
			KillOffset.Z -= 110.0f;

			AttackableInterface->OnAttacked(KillOffset, 160.0f);

			if (AbilitySystemComponent)
			{
				FGameplayEventData Payload;
				Payload.Instigator = this;
				Payload.Target = OtherActor;

				FGameplayTag KillTag = FGameplayTag::RequestGameplayTag(FName("AI.Ability.Behavior.Attack"));

				AbilitySystemComponent->HandleGameplayEvent(KillTag, &Payload);
			}
		}
	}
}
