// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PGCharacterBase.h"
#include "GameplayTagContainer.h"
#include "PGPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

struct FOnAttributeChangeData;

class UPGInventoryComponent;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

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
	
protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

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

///
///*********	Gameplay Ability System ******************
/// 	
public:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	FORCEINLINE FGameplayTagContainer GetInteractTag() const { return InteractTag; }

private:
	void InitAbilitySystemComponent();

protected:

	//Gameplay ability tags
	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer SprintTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability | Tags")
	FGameplayTagContainer InteractTag;

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

protected:
	//The interactive actor currently watching
	TObjectPtr<AActor> InteractionTargetActor;

///
///		UI and Components
/// 
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPGInventoryComponent> InventoryComponent;
};
