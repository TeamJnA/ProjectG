// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PGPlayerState.generated.h"

class UPGAbilitySystemComponent;
class UPGAttributeSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerStateUpdatedDelegate);

UCLASS()
class PROJECTG_API APGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APGPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual UPGAttributeSet* GetAttributeSet() const;
	
	bool IsHost() const { return bIsHost; }
	void SetHost(bool bInIsHost) { bIsHost = bInIsHost; OnRep_PlayerStateUpdated(); }

	bool HasFinishedGame() const { return bHasFinishedGame; }
	void SetHasFinishedGame(bool bInHasFinishedGame) { bHasFinishedGame = bInHasFinishedGame; OnRep_PlayerStateUpdated(); }

	bool IsDead() const { return bIsDead; }
	void SetIsDead(bool bInIsDead) { bIsDead = bInIsDead; OnRep_PlayerStateUpdated(); }

	bool IsReadyToReturnLobby() const { return bIsReadyToReturnLobby; }
	void SetReadyToReturnLobby(bool bInIsReadyToReturnLobby) { bIsReadyToReturnLobby = bInIsReadyToReturnLobby; OnRep_PlayerStateUpdated(); }

	FOnPlayerStateUpdatedDelegate OnPlayerStateUpdated;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<UPGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UPGAttributeSet> AttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerStateUpdated)
	bool bIsHost = false;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerStateUpdated)
	bool bHasFinishedGame = false;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerStateUpdated)
	bool bIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerStateUpdated)
	bool bIsReadyToReturnLobby = false;

	UFUNCTION()
	void OnRep_PlayerStateUpdated();

private:
};
