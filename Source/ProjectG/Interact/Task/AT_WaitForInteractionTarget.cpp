// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Task/AT_WaitForInteractionTarget.h"
#include "Interface/InteractableActorInterface.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"

UAT_WaitForInteractionTarget* UAT_WaitForInteractionTarget::WaitForInteractionTarget
(UGameplayAbility* OwningAbility, UCameraComponent* ActorCameraComponent, bool ShowDebug
, float TraceRate, float TraceRange)
{
	UAT_WaitForInteractionTarget* MyObj = NewAbilityTask<UAT_WaitForInteractionTarget>(OwningAbility);
	MyObj->CameraComponent = ActorCameraComponent;
	MyObj->InteractableTraceRate = TraceRate;
	MyObj->InteractableTraceRange = TraceRange;
	MyObj->ShowDebug = ShowDebug;

	return MyObj;
}

void UAT_WaitForInteractionTarget::Activate()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::TraceToFindInteractable, InteractableTraceRate, true);
}

void UAT_WaitForInteractionTarget::TraceToFindInteractable()
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot found ASC in WaitForInteractionTarget"));
		return;
	}

	//Do Linetrace and find interactable actor

	//Add avatar actor not to trace
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!AvatarActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot found avatar actor in WaitForInteractionTarget"));
		return;
	}
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(AvatarActor);

	FVector TraceStartLocation = CameraComponent->GetComponentLocation();
	FVector TraceStartDirection = CameraComponent->GetForwardVector();

	//Do linetrace and show debug
	FHitResult HitResult;
	FVector TraceEndLocation = TraceStartLocation + TraceStartDirection * InteractableTraceRange;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, TraceParams);
	if (ShowDebug) {
		DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Green, false, 0.5f);
	}

	//Broadcast trace result to interact ability.
	if(bHit && HitResult.GetActor())
	{
		//Check actor Interface which makes actor interactable.
		IInteractableActorInterface* InteractInterface = Cast<IInteractableActorInterface>(HitResult.GetActor());

		//If there's new interactable target
		if (InteractInterface && (!PreviousTargetActor.IsValid() || PreviousTargetActor != HitResult.GetActor()) )
		{
			PreviousTargetActor = HitResult.GetActor();
			InteractionTarget.Broadcast(PreviousTargetActor.Get());
		}
		//If the new actor doesn't have InteractInterface
		else if (!InteractInterface && PreviousTargetActor.IsValid())
		{
			PreviousTargetActor.Reset();
			InteractionTarget.Broadcast(PreviousTargetActor.Get());
		}
	}
	//If there's no HitResult but task has previous actor
	else
	{
		if (PreviousTargetActor.IsValid())
		{
			PreviousTargetActor.Reset();
			InteractionTarget.Broadcast(PreviousTargetActor.Get());
		}
	}
}

void UAT_WaitForInteractionTarget::OnDestroy(bool AbilityIsEnding)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle);
	}

	Super::OnDestroy(AbilityIsEnding);
}
