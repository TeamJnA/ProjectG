// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PGCharacterBase.h"

#include "GameplayTagContainer.h"
#include "Type/CharacterTypes.h"
#include "Interface/AttackableTarget.h"
#include "Interface/InteractableActorInterface.h"
#include "Interface/HandItemInterface.h"
#include "Interface/GimmickTargetInterface.h"
#include "GenericTeamAgentInterface.h"

#include "PGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

struct FOnAttributeChangeData;

class UPGInventoryComponent;
class USpotLightComponent;

class APGTriggerGimmickMannequin;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStareTargetUpdate, AActor*, InteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAutomatedMovementCompleted);

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPlayerCharacter : public APGCharacterBase, public IAttackableTarget, public IHandItemInterface,
	public IInteractableActorInterface, public IGenericTeamAgentInterface, public IGimmickTargetInterface
{
	GENERATED_BODY()
	
public:
	APGPlayerCharacter();

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	/** First Person camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Headlight, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> HeadlightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Headlight, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpotLightComponent> HeadlightLight;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	//ItemSlot Actions. Make delegate to call BindAction function with parameter
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UInputAction>> ChangeItemSlotAction;

	//GameplayAbility Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HeadLightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DropItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseRightAction;

	// debug
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DebugDecreaseSanityAction;

	UPROPERTY(EditDefaultsOnly, Category = "Sanity")
	TSubclassOf<UGameplayEffect> SanityDecreaseEffect;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface~
	virtual FGenericTeamId GetGenericTeamId() const override;
	// ~IGenericTeamAgentInterface

protected:
	virtual void BeginPlay() override;
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// Update server camera pitch rotation. It sends client camera to server.
	UFUNCTION(Server, Reliable)
	void Server_SendCameraRotation(FRotator NewRotation);

	/** Called for Input ability Actions. Sprint, interaction*/
	void StartInputActionByTag(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag);
	void StopInputActionByTag(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag);

	/** Called for Changing inventory currecnt slot by number */
	void ChangingItemSlot(const FInputActionValue& Value, int32 NumofSlot);

	void OnDebugDecreaseSanity(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_Debug_DecreaseSanity();

protected:
	// Controller가 OnPossess할 때, Pawn의 PawnClientRestart를 호출하고 거기서 SetupPlayerInputComponent 호출
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }


///
///*********	Player State (Dead or Alive) ******************
/// 	
	

	// IAttackableTarget~
	// If player is not dead(check by Gamplay Tag), is valid target is true.
	bool IsValidAttackableTarget() const;

	void OnAttacked(const FVector InstigatorHeadLocation, const float EnemyCharacterDistance);

	void OnAttackFinished();
	// ~IAttackableTarget

	UFUNCTION(Client, Reliable)
	void Client_OnAttacked(FVector NewLocation, FRotator NewRotation);

	void OnDeadTagChanged(const FGameplayTag Tag, int32 NewCount);

	/// <summary>
	/// Server-only death handling function
	/// When the player dies, they drop their items and switch to a ragdoll state.
	/// </summary>
	void OnPlayerDeathAuthority();

	/// <summary>
	/// Client-only death handling function
	/// The camera slowly pulls back from the character, enabling spectator mode.
	/// </summary>
	void OnPlayerDeathLocally();

	// Ragdolls 
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsRagdoll, VisibleAnywhere, BlueprintReadWrite, Category = Ragdoll)
	bool bIsRagdoll = false;

	// IInteractableActorInterface ~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	//~ IInteractableActorInterface end

	void OnRevive();

	FORCEINLINE APlayerState* GetDeadPlayerState() const { return DeadPlayerState; }

protected:
	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> DeadPlayerState;

	UFUNCTION()
	void OnRep_IsRagdoll();

///
///*********	Gameplay Ability System ******************
/// 	
public:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	FORCEINLINE FGameplayTagContainer GetInteractTag() const { return InteractTag; }

	void ActivateAbilityByTag(const FGameplayTagContainer Tag);

private:
	void InitAbilitySystemComponent();

protected:

	//Gameplay ability tags
	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer SprintTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer CrouchTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer InteractTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer HeadLightTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer MouseLeftTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer MouseRightTag;

	UFUNCTION(Server, Reliable)
	void AddTagToCharacter(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag);

	UFUNCTION(Server, Reliable)
	void RemoveTagFromCharacter(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag);

// --------------Movement parts------------------
protected:
	//Movement speed changed delegate
	FDelegateHandle MovementSpeedChangedDelegateHandle;

public:
	//Bind with MovementSpeedChangedDelegateHandle. 
	//If attribute speed is changed, change character speed
	void OnMovementSpeedChanged(const FOnAttributeChangeData& Data);
	

// --------------Interaction parts------------------
	FORCEINLINE TObjectPtr<AActor> GetInteractionTargetActor() {return InteractionTargetActor;}

	//Cache item to interact
	UFUNCTION()
	void CacheInteractionTarget(AActor* CacheInteractTarget);

	//Hand Actions
	TArray<TObjectPtr<UAnimMontage>> HandActionAnimMontages;

	EHandActionMontageType HandActionMontageType;

	TObjectPtr<UAnimMontage> GetHandActionAnimMontages();

	void SetHandActionAnimMontage(EHandActionMontageType _HandActionMontageType);

	void PlayHandActionAnimMontage(EHandActionMontageType _HandActionMontageType);

protected:
	//The interactive actor currently watching
	TObjectPtr<AActor> InteractionTargetActor;

// ------------ HeadLight --------------------
public:
	void Multicast_SetHeadlightState(bool InbIsFlashlightOn);
	FORCEINLINE FName GetHeadlightSoundName() const { return HeadlightSoundName; }

protected:
	void ToggleHeadLight();

	UPROPERTY(EditDefaultsOnly, Category = "Sound | Headlight")
	FName HeadlightSoundName = FName("SFX_Headlight");

///
///********* Item system ******************
/// 
public:
	void EquipCurrentInventoryItem();

	void AttachMeshOnHand();

	void DetachMeshOnHand();

	void RemoveItemFromInventory();

	UFUNCTION(Server, Reliable)
	void DropItem();

	// IHandItemInterface 
	void SetItemMesh(const bool bIsVisible);
	void SetRightHandIK();
	// ~ IHandItemInterface

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TObjectPtr<UStaticMeshComponent> EquippedItemMesh;
///
///********* UI and Components ******************
///

// --------------Inventory parts------------------
public:
	UPGInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGInventoryComponent> InventoryComponent;

// UI
public:
	void InitHUD();

	UFUNCTION(Client, Reliable)
	void Client_PlayerStareAtTarget(AActor* TargetActor);

	UFUNCTION(Client, Reliable)
	void Client_DisplayInteractionFailedMessage(const FText& Message);

	UPROPERTY()
	FOnStareTargetUpdate OnStareTargetUpdate;

	UFUNCTION(Client, Reliable)
	void Client_InitScoreBoardWidget();

	UFUNCTION(Client, Reliable)
	void Client_UpdateInteractionProgress(float Progress);

protected:
	TObjectPtr<AActor> StaringTargetActor = nullptr;

public:
	UPROPERTY()
	FOnAutomatedMovementCompleted OnAutomatedMovementCompleted;

	void StartAutomatedMovement(const FVector& TargetLocation);

private:
	// 자동 이동 관련 변수들
	bool bIsMovingAutomated = false;
	FVector AutomatedMoveTarget;
	FTimerHandle AutomatedMoveTimer;

	void UpdateAutomatedMovement();

// Gimmick
public:
	// IGimmickTargetInterface~
	virtual void RequestApplyGimmickEffect(TSubclassOf<UGameplayEffect> EffectClass) override;
	virtual float GetSanityValue() const override;
	// ~IGimmickTargetInterface

	UFUNCTION(Server, Unreliable)
	void Server_ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass);
};
