// Fill out your copyright notice in the Description page of Project Settings.


#include "Interact/Task/AT_WaitForInteractionTarget.h"
#include "Interface/InteractableActorInterface.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"

UAT_WaitForInteractionTarget* UAT_WaitForInteractionTarget::WaitForInteractionTarget(UGameplayAbility* OwningAbility, 
	UCameraComponent* ActorCameraComponent,
	APawn* InAvatarPawn,
	bool ShowDebug,
	float TraceRate,
	float TraceRange)
{
	UAT_WaitForInteractionTarget* MyObj = NewAbilityTask<UAT_WaitForInteractionTarget>(OwningAbility);
	MyObj->CameraComponent = ActorCameraComponent;
	MyObj->AvatarPawn = InAvatarPawn;
	MyObj->InteractTraceRate = TraceRate;
	MyObj->InteractTraceRange = TraceRange;
	MyObj->ShowDebug = ShowDebug;

	return MyObj;
}

void UAT_WaitForInteractionTarget::Activate()
{
	bIsPreviousTargetValid = false;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::TraceToFindInteractable, InteractTraceRate, true);
}

void UAT_WaitForInteractionTarget::TraceToFindInteractable()
{
	/*
	*  Do Linetrace and find interactable actor
	*/

	// Add avatar actor not to trace
	if (!AvatarPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot found avatar actor in WaitForInteractionTarget"));
		return;
	}

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(AvatarPawn);

	const FVector TraceStartLocation = CameraComponent->GetComponentLocation();
	const FVector TraceDirection = AvatarPawn->GetControlRotation().Vector();
	const FVector TraceEndLocation = TraceStartLocation + (TraceDirection * InteractTraceRange);

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStartLocation, TraceEndLocation, ECC_Visibility, TraceParams);

#if WITH_EDITOR
	if (ShowDebug)
	{
		DrawDebugLine(GetWorld(), TraceStartLocation, TraceEndLocation, FColor::Green, false, 0.5f);
	}
#endif

	// Broadcast trace result to interact ability.
	if (bHit && HitResult.GetActor())
	{
		// Check if the actor implements IInteractableActorInterface that allows interaction.
		IInteractableActorInterface* InteractInterface = Cast<IInteractableActorInterface>(HitResult.GetActor());

		// If there's new interactable target, Broadcast the target.
		//if (InteractInterface && (!PreviousTargetActor.IsValid() || PreviousTargetActor != HitResult.GetActor()) )
		if (InteractInterface)
		{
			bIsPreviousTargetValid = true;
			PreviousTargetActor = HitResult.GetActor();
			InteractionTarget.Broadcast(PreviousTargetActor.Get());
		}
		// If the new actor doesn't have InteractInterface, broadcast NullPtr.
		else
		{
			bIsPreviousTargetValid = false;
			PreviousTargetActor.Reset();
			InteractionTarget.Broadcast(PreviousTargetActor.Get());
		}
	}
	// If there's no HitResult but task has previous actor, broadcast NullPtr.
	else if (bIsPreviousTargetValid)
	{
		bIsPreviousTargetValid = false;
		PreviousTargetActor.Reset();
		InteractionTarget.Broadcast(PreviousTargetActor.Get());
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
