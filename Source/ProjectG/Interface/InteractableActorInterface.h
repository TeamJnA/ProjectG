// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableActorInterface.generated.h"

class UGameplayAbility;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
	Instant, // 즉시 발동(Item)
	Hold // 홀드(Door, ExitDoor)
};

USTRUCT(BlueprintType)
struct FInteractionPromptInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UMaterialInterface> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Icon != nullptr"))
	FVector2D IconSize = FVector2D(32.0f, 32.0f);

	bool IsValid() const { return Icon != nullptr; }
};

USTRUCT(BlueprintType)
struct FInteractionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInteractionType InteractionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HoldStartTime;

	FInteractionInfo()
		: InteractionType(EInteractionType::Instant)
		, HoldDuration(0.0f)
		, HoldStartTime(0.0f)
	{}

	FInteractionInfo(EInteractionType InType, float InDuration = 0.0f, float InHoldStartTime = 0.0f)
		: InteractionType(InType)
		, HoldDuration(InDuration)
		, HoldStartTime(InHoldStartTime)
	{}
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTG_API IInteractableActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const = 0;
	virtual void HighlightOn() const = 0;
	virtual void HighlightOff() const = 0;
	virtual FInteractionInfo GetInteractionInfo() const = 0;
	virtual FText GetInteractionText() const { return FText::GetEmpty(); }
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const
	{
		return true;
	}
	virtual void InteractionFailed() {}
};
