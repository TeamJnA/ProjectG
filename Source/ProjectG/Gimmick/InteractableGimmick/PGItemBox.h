// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGItemBox.generated.h"

#define LOCTEXT_NAMESPACE "PGInteraction"

class UPGItemData;

/**
 * 
 */
UCLASS()
class PROJECTG_API APGItemBox : public APGInteractableGimmickBase
{
	GENERATED_BODY()
	
public:
	APGItemBox();

	// IInteractableActorInterface~
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	// ~IInteractableActorInterface

	virtual void GimmickInteract(AActor* Investigator) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TObjectPtr<UPGItemData> ItemToDispense;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FText TakeText = LOCTEXT("ItemBox_Take", "Take");
};

#undef LOCTEXT_NAMESPACE
