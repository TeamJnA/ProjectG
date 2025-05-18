// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PGCharacterBase.generated.h"

//Forward declarations
class UPGAbilitySystemComponent;
class UGameplayAbility;
class UPGAttributeSet;
class UGameplayEffect;

UCLASS()
class PROJECTG_API APGCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APGCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UPGAttributeSet* GetAttributeSet() const;

public:
	void GiveDefaultAbilities();
	void InitDefaultAttributes() const;
	void GiveAndActivatePassiveEffects();

protected:
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
public:	
};
