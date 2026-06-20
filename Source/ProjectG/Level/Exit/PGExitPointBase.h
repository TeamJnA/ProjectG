// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableActorInterface.h"
#include "Type/PGGameTypes.h"
#include "PGLogChannels.h"
#include "Camera/CameraComponent.h"
#include "PGExitPointBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExitLockStateChanged, class APGExitPointBase*, ExitActor);

UCLASS()
class PROJECTG_API APGExitPointBase : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGExitPointBase();

	virtual void BeginPlay() override;

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	virtual void NotifyInteractionAttempted(ACharacter* InteractingPlayer) override;
	// ~IInteractableActorInterface

	// If true, remove item from inventory
	virtual bool Unlock(AActor* Investigator);
	virtual TSet<FName> GetUnlockedItemIds() const { return TSet<FName>(); }
	virtual bool IsExitDepleted() const { return false; }

	FORCEINLINE FVector GetCameraLocation() const { return ExitCamera->GetComponentLocation(); }
	FORCEINLINE FRotator GetCameraRoation() const { return ExitCamera->GetComponentRotation(); }
	FORCEINLINE int32 GetLinkedSpeciesKey() const { return LinkedSpeciesKey; }

	FOnExitLockStateChanged OnExitLockStateChanged;

protected:
	UFUNCTION()
	void PlaySound(const FName& SoundName, const FVector& SoundLocation, AActor* Investigator = nullptr);

	UFUNCTION()
	void PlaySoundPlayers(const FName& SoundName, const FVector& SoundLocation);

	UFUNCTION()
	void OnEscapeStart(AActor* EscapeStartActor, EExitPointType ExitPointType = EExitPointType::IronDoor);

	void DropEscaperItems(AActor* EscapeStartActor);

	void RegisterExitCamera();

	void BroadcastLockStateChanged();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> ExitCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ExitDrop")
	TObjectPtr<USceneComponent> ItemDropPointOnExit;

	int32 RegistrationRetries = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	int32 LinkedSpeciesKey = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Helper")
	int32 InteractionDiscoveryScore = 100;

	EExitPointType ExitPointType;
};
