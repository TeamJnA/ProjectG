// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PGAttributeSet.generated.h"

//Helper macro to define the accessors for an attribute. Make getter, setter... functions
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
//ATTRIBUTE_ACCESSORS(UPGAtrributeSet, Stamina)
/**
 * 
 */

UCLASS()
class PROJECTG_API UPGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPGAttributeSet();

	// Registers variables for replication so clients automatically receive updates from the server.  
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Before attribute value is changed, pre modify value
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//After attribute value is changed, doing additional processing
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UPGAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UPGAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Sanity, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Sanity;
	ATTRIBUTE_ACCESSORS(UPGAttributeSet, Sanity);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSanity, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxSanity;
	ATTRIBUTE_ACCESSORS(UPGAttributeSet, MaxSanity);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UPGAttributeSet, MovementSpeed);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

	UFUNCTION()
	void OnRep_Sanity(const FGameplayAttributeData& OldSanity) const;

	UFUNCTION()
	void OnRep_MaxSanity(const FGameplayAttributeData& OldMaxSanity) const;

	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed) const;

};
