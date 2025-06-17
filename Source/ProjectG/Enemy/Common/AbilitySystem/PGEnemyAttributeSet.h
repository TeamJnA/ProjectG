// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PGEnemyAttributeSet.generated.h"

//Helper macro to define the accessors for an attribute. Make getter, setter... functions
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
//ATTRIBUTE_ACCESSORS(UPGAtrributeSet, Stamina)

UCLASS()
class PROJECTG_API UPGEnemyAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPGEnemyAttributeSet();
	
	// Registers variables for replication so clients automatically receive updates from the server.  
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Before attribute value is changed, pre modify value
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//After attribute value is changed, doing additional processing
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;






	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MovementSpeed, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UPGEnemyAttributeSet, MovementSpeed);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMovementSpeed, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxMovementSpeed;
	ATTRIBUTE_ACCESSORS(UPGEnemyAttributeSet, MaxMovementSpeed);



	UFUNCTION()
	void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed) const;

	UFUNCTION()
	void OnRep_MaxMovementSpeed(const FGameplayAttributeData& OldMaxMovementSpeed) const;

};
