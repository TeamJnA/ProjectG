// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/InteractableActorInterface.h"

#include "PGInteractableGimmickBase.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;
class APGSoundManager;

UCLASS(Abstract)
class PROJECTG_API APGInteractableGimmickBase : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGInteractableGimmickBase();

	virtual void GimmickInteract();

	void InitSoundManager();

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	// ~IInteractableActorInterface

	void SelfHighlightOn();
	void SelfHighlightOff();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PlayLocalSound(FName SoundName, FVector SoundLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InteractAbility")
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(Replicated)
	TObjectPtr<APGSoundManager> SoundManager;
};
