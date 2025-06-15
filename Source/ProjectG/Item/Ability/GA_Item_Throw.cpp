// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Ability/GA_Item_Throw.h"
#include "Character/Ability/Task/AT_PGWaitGameplayTagReAdded.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "AbilitySystemComponent.h"
#include "Character/PGPlayerCharacter.h"

DEFINE_LOG_CATEGORY(LogAbility);

UGA_Item_Throw::UGA_Item_Throw()
{
	MouseLeftTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseLeft"));
	MouseRightTag = FGameplayTag::RequestGameplayTag(FName("Input.MouseRight"));

	HandActionTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	bThrowReady = false;

	// This ability is activated in server.
	// If activated with local predict, there is an issue with spawn projectile item.
	// NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> ThrowAnimMontageRef(TEXT("/Game/ProjectG/Character/Animation/Throw/AM_Throw.AM_Throw"));
	if (ThrowAnimMontageRef.Object){
		ThrowAnimMontage = ThrowAnimMontageRef.Object;
	}
	else{
		UE_LOG(LogAbility, Warning, TEXT("Anim montage not found in GA_Item_Throw."));
	}
}

void UGA_Item_Throw::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogAbility, Log, TEXT("%s | %s ability activated."), *GetClass()->GetName(), *GetName());
	bThrowReady = false;

	// Wait to input mouse left or mouse right button.
	// Right : Draw throwable item trajactory.
	// Left : Throw item.

	UAT_PGWaitGameplayTagReAdded* WaitForRightMouseTag = UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(
		this, 
		MouseRightTag);
	WaitForRightMouseTag->Added.AddDynamic(this, &UGA_Item_Throw::MouseRight);
	WaitForRightMouseTag->ReadyForActivation();
	
	UAT_PGWaitGameplayTagReAdded* WaitForLeftMouseTag = UAT_PGWaitGameplayTagReAdded::WaitGameplayTagReAdded(
		this,
		MouseLeftTag);
	WaitForLeftMouseTag->Added.AddDynamic(this, &UGA_Item_Throw::MouseLeft);
	WaitForLeftMouseTag->ReadyForActivation();
}

void UGA_Item_Throw::MouseLeft()
{
	UE_LOG(LogAbility, Log, TEXT("MouseLeft Input. Start to throw item! %s | %s"), *GetClass()->GetName(), *GetName());

	//Throwing Start. Check HandAction.Lock tag.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}

	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s during hand action."), *GetName());
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(HandActionTag);
	//AbilitySystemComponent->AddReplicatedLooseGameplayTag(HandActionTag);

	//Play Throw anim montage
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			1.0f,
			TEXT("Throw")
		);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_Item_Throw::ThrowItemComplete);
	MontageTask->OnBlendOut.AddDynamic(this, &UGA_Item_Throw::ThrowItemComplete);
	MontageTask->ReadyForActivation();

	// Spawn item actor only on the server.
	if (HasAuthority(&CurrentActivationInfo))
	{
		SpawnProjectileActor();
	}
}

void UGA_Item_Throw::ThrowItemComplete()
{
	UE_LOG(LogAbility, Log, TEXT("Throw item completely! %s"), *GetName());

	//Unlock handaction
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s"), *GetName());
		return;
	}
	AbilitySystemComponent->RemoveLooseGameplayTag(HandActionTag);
	// AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(HandActionTag);

	// Wait for tag removed on client and clear this ability in the inventory component on server.
	// If clear ability directly, remove tag does not replicated in client by clear ability.
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		UE_LOG(LogAbility, Log, TEXT("Wait for HandActionTag removed in client of %s"), *GetName());

		UAbilityTask_WaitGameplayTagRemoved* WaitHandActionTagRemoved = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			HandActionTag );
		WaitHandActionTagRemoved->Removed.AddDynamic(this, &UGA_Item_Throw::RemoveItem);
		WaitHandActionTagRemoved->ReadyForActivation();
	}

	UE_LOG(LogAbility, Log, TEXT("IsLocallyControlled: %s"), IsLocallyControlled() ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogAbility, Log, TEXT("AvatarActor HasAuthority: %s"), GetAvatarActorFromActorInfo()->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogAbility, Log, TEXT("OwningActor HasAuthority: %s"), GetOwningActorFromActorInfo()->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));

	ENetRole OwningActorNetRole = GetOwningActorFromActorInfo()->GetLocalRole();

	switch (OwningActorNetRole)
	{
	case ROLE_None:
		UE_LOG(LogTemp, Warning, TEXT("NetRole: None"));
		break;
	case ROLE_SimulatedProxy:
		UE_LOG(LogTemp, Warning, TEXT("NetRole: Simulated Proxy"));
		break;
	case ROLE_AutonomousProxy:
		UE_LOG(LogTemp, Warning, TEXT("NetRole: Autonomous Proxy"));
		break;
	case ROLE_Authority:
		UE_LOG(LogTemp, Warning, TEXT("NetRole: Authority"));
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("NetRole: Unknown"));
		break;
	}

	
	ENetMode NetMode = GetWorld()->GetNetMode();

	switch (NetMode)
	{
	case NM_Standalone:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Standalone"));
		break;
	case NM_DedicatedServer:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Dedicated Server"));
		break;
	case NM_ListenServer:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Listen Server"));
		break;
	case NM_Client:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Client"));
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Unknown"));
		break;
	}
}
// RemoveItem의 Server함수를 만드는 건 어떨까? 그게 좋을거 같음.
// Server_RemoveItem 이런식으로 하면 좋을 거 같음.
// 그러면 IsLocallyControlled : true, HasAuthority : true (리슨서버의 호스트 캐릭터) 바로 RemoveItem 호출.
// HasAuthority : false. (클라이언트) 인 경우에는 client의 handaction.Lock Tag 제거 기다린 후 Server_RemoveItem 호출.
void UGA_Item_Throw::RemoveItem_Implementation()
{
	UE_LOG(LogAbility, Log, TEXT("HandActionTag removed in client of %s. Now clear ability and delete the item by requesting to inventory.[1]"), *GetName());
	UE_LOG(LogAbility, Log, TEXT("HasAuthority: %s"), GetAvatarActorFromActorInfo()->HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"));
	UE_LOG(LogAbility, Log, TEXT("IsLocallyControlled: %s"), IsLocallyControlled() ? TEXT("TRUE") : TEXT("FALSE"));

	ENetMode NetMode = GetWorld()->GetNetMode();

	switch (NetMode)
	{
	case NM_Standalone:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Standalone"));
		break;
	case NM_DedicatedServer:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Dedicated Server"));
		break;
	case NM_ListenServer:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Listen Server"));
		break;
	case NM_Client:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Client"));
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("NetMode: Unknown"));
		break;
	}

	// Remove item. Inventory is replicated, so remove item also only on the server.
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		UE_LOG(LogAbility, Log, TEXT("HandActionTag removed in client of %s. Now clear ability and delete the item by requesting to inventory.[2]"), *GetName());
		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		APGPlayerCharacter* PGPC = Cast<APGPlayerCharacter>(AvatarActor);
		if (!PGPC) {
			UE_LOG(LogAbility, Warning, TEXT("Cannot found avatar actor in RemoveItem %s"), *GetName());
			return;
		}

		PGPC->RemoveItemFromInventory();
	}
}

void UGA_Item_Throw::MouseRight()
{
	//Cannot do ThrowReady while HandAction.
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s ::RightInputCanceled"), *GetName());
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s ::RightInputCanceled during hand action."), *GetName());
		return;
	}

	UE_LOG(LogAbility, Log, TEXT("MouseRight Input. Ready to throw. %s | %s"), *GetClass()->GetName(), *GetName());
	bThrowReady = true;
	
	//Wait at throw ready
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			0.0f,
			TEXT("Throw")
		);

	//Do not into RightInputCanceled by setting RightClickCanceled.
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Item_Throw::ThrowReadyCanceled);
	MontageTask->ReadyForActivation();

	///
	/// Draw throw prediction line.
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 
	/// 

	// Wait for tag removed : Right Input.
	// Play anim montage to return hand to default position.
	UAbilityTask_WaitGameplayTagRemoved* TagRemovedTask =
		UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			MouseRightTag
		);
	TagRemovedTask->Removed.AddDynamic(this, &UGA_Item_Throw::RightInputCanceled);
	TagRemovedTask->ReadyForActivation();
}

void UGA_Item_Throw::RightInputCanceled()
{
	if (bThrowReady == false)
		return;
	
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogAbility, Warning, TEXT("AbilitySystemComponent cannot found in %s ::RightInputCanceled"), *GetName());
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionTag))
	{
		UE_LOG(LogAbility, Log, TEXT("Cannot do %s ::RightInputCanceled during hand action."), *GetName());
		return;
	}

	// Play anim montage to return hand to default position.
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			TEXT("FullThrowAnimMontage"),
			ThrowAnimMontage,
			-1.0f,
			TEXT("ThrowStart")
		);
	MontageTask->ReadyForActivation();
}

//This fuction implemented in child classes.
//It is implemented differently depending on which actor item to spawn.
void UGA_Item_Throw::SpawnProjectileActor()
{
	UE_LOG(LogAbility, Log, TEXT("SpawnProjectileActor"));
}

void UGA_Item_Throw::ThrowReadyCanceled()
{
	bThrowReady = false;
}

void UGA_Item_Throw::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}