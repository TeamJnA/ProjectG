// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PGBlindAttributeSet.generated.h"

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
class PROJECTG_API UPGBlindAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UPGBlindAttributeSet();

	// Registers variables for replication so clients automatically receive updates from the server.  
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Before attribute value is changed, pre modify value
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//After attribute value is changed, doing additional processing
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;




	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HearingRange, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData HearingRange;
	ATTRIBUTE_ACCESSORS(UPGBlindAttributeSet, HearingRange);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHearingRange, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxHearingRange;
	ATTRIBUTE_ACCESSORS(UPGBlindAttributeSet, MaxHearingRange);



	UFUNCTION()
	void OnRep_HearingRange(const FGameplayAttributeData& OldHearingRange) const;

	UFUNCTION()
	void OnRep_MaxHearingRange(const FGameplayAttributeData& OldMaxHearingRange) const;






};
