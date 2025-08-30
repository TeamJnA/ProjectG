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
	
	const bool IsHost() const { return bIsHost; }
	void SetHost(bool _bIsHost) { bIsHost = _bIsHost; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<UPGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPGAttributeSet> AttributeSet;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
	bool bIsHost = false;

private:
};
