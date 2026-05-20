// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Interface/InteractableActorInterface.h"

#include "PGLobbyDoor.generated.h"

UCLASS()
class PROJECTG_API APGLobbyDoor : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGLobbyDoor();

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	// ~IInteractableActorInterface

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DoorMesh")
	TObjectPtr<UStaticMeshComponent> Mesh1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InteractAbility", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	TObjectPtr<UMaterialInterface> HostOnlyIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI Icons")
	FVector2D HostOnlyIconSize = FVector2D(70.0f, 70.0f);

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText StartGameText = FText::FromString(TEXT("Start Game"));
};
