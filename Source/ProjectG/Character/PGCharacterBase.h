// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PGCharacterBase.generated.h"

// Ability
class UPGAbilitySystemComponent;
class UGameplayAbility;
class UPGAttributeSet;
class UGameplayEffect;

// Sound
class UPGSoundManagerComponent;
class APGSoundManager;

UCLASS()
class PROJECTG_API APGCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APGCharacterBase();

// -----------Ability-------------
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UPGAttributeSet* GetAttributeSet() const;

	FORCEINLINE UPGSoundManagerComponent* GetSoundManagerComponent() const { return SoundManagerComponent; }

protected:
	void GiveDefaultAbilities();
	void InitDefaultAttributes() const;
	void GiveAndActivatePassiveEffects();

	UPROPERTY()
	TObjectPtr<UPGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPGAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<TSubclassOf<UGameplayEffect>> PassiveEffects;

// -----------Sound-------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Sound, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPGSoundManagerComponent> SoundManagerComponent;

public:
	void InitSoundManager(APGSoundManager* SoundManagerRef);
};
