// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PGPlayerState.generated.h"

class UPGAbilitySystemComponent;
class UPGAttributeSet;

UCLASS()
class PROJECTG_API APGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APGPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UPGAttributeSet* GetAttributeSet() const;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly)
	bool bIsReady = false;

	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bReady);

protected:
	UFUNCTION()	
	void OnRep_IsReady();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<UPGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPGAttributeSet> AttributeSet;
};
