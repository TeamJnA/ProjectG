// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableActorInterface.h"
#include "Type/PGGameTypes.h"
#include "PGLogChannels.h"
#include "Camera/CameraComponent.h"
#include "PGExitPointBase.generated.h"

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
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	// If true, remove item from inventory
	virtual bool Unlock();

	FORCEINLINE FVector GetCameraLocation() const { return ExitCamera->GetComponentLocation(); }
	FORCEINLINE FRotator GetCameraRoation() const { return ExitCamera->GetComponentRotation(); }

protected:
	UFUNCTION()
	void PlaySound(const FName& SoundName, const FVector& SoundLocation);

	UFUNCTION()
	void PlaySoundPlayers(const FName& SoundName, const FVector& SoundLocation);

	UFUNCTION()
	void OnEscapeStart(AActor* EscapeStartActor, EExitPointType ExitPointType = EExitPointType::IronDoor);

	void RegisterExitCamera();
	int32 RegistrationRetries = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> ExitCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	EExitPointType ExitPointType;
};
