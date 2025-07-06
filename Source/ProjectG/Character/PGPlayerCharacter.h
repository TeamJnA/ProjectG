// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PGCharacterBase.h"
#include "GameplayTagContainer.h"
#include "Type/CharacterTypes.h"
#include "PGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

struct FOnAttributeChangeData;

class UPGInventoryComponent;
class USpotLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStareTargetUpdate, AActor*, InteractableActor);

class UPGSoundManagerComponent;
class APGSoundManager;

//TEST
class APGDoor1;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGPlayerCharacter : public APGCharacterBase
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

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

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
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DropItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MouseRightAction;
	
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

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE class UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

///
///*********	Gameplay Ability System ******************
/// 	
public:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	FORCEINLINE FGameplayTagContainer GetInteractTag() const { return InteractTag; }

	void ActivateAbilityByTag(FGameplayTagContainer Tag);

private:
	void InitAbilitySystemComponent();

protected:

	//Gameplay ability tags
	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer SprintTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer InteractTag;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	TObjectPtr<USceneComponent> ItemSocket;

///
///********* UI and Components ******************
///

// --------------Inventory parts------------------
public:
	UPGInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGInventoryComponent> InventoryComponent;

	TObjectPtr<AActor> StaringTargetActor = nullptr;

public:
	void InitHUD();

	UFUNCTION(Client, Reliable)
	void Client_PlayerStareAtTarget(AActor* TargetActor);

	UPROPERTY()
	FOnStareTargetUpdate OnStareTargetUpdate;

	void MC_SetFlashlightState(bool _bIsFlashlightOn);

// -----------Sound parts-------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Sound, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGSoundManagerComponent> SoundManagerComponent;

public:
	void InitSoundManager(APGSoundManager* SoundManagerRef);

// UI
	void InitHUD() const;


// TEST
	void TEST_Interact();

	UFUNCTION(Server, Reliable)
	void TEST_Server_Interact(APGDoor1* Door);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TEST_DoorInteract;
	
};
