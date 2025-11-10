// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "PGDeafAttributeSet.generated.h"

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
class PROJECTG_API UPGDeafAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPGDeafAttributeSet();

	// Registers variables for replication so clients automatically receive updates from the server.  
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Before attribute value is changed, pre modify value
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//After attribute value is changed, doing additional processing
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;




	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SightRadius, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData SightRadius;
	ATTRIBUTE_ACCESSORS(UPGDeafAttributeSet, SightRadius);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSightRadius, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxSightRadius;
	ATTRIBUTE_ACCESSORS(UPGDeafAttributeSet, MaxSightRadius);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SightAngle, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData SightAngle;
	ATTRIBUTE_ACCESSORS(UPGDeafAttributeSet, SightAngle);



	UFUNCTION()
	void OnRep_SightRadius(const FGameplayAttributeData& OldSightRadius) const;

	UFUNCTION()
	void OnRep_MaxSightRadius(const FGameplayAttributeData& OldMaxSightRadius) const;

	UFUNCTION()
	void OnRep_SightAngle(const FGameplayAttributeData& OldSightAngle) const;
	
};
