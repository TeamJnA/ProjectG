// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractableActorInterface.h"
#include "Interface/HoldInteractProgressHandler.h"
#include "PGFuseBox.generated.h"

class APGItemActor;
class UPGItemData;

UENUM(BlueprintType)
enum class EFuseBoxState : uint8
{
	Closed,
	Opened,
	Empty
};

UCLASS()
class PROJECTG_API APGFuseBox : public AActor, public IInteractableActorInterface, public IHoldInteractProgressHandler
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGFuseBox();

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const override;
	// ~IInteractableActorInterface

	// IHoldInteractProgressHandler~
	virtual void UpdateHoldProgress(float Progress) override;
	virtual void StopHoldProress() override;
	// ~IHoldInteractProgressHandler

	void OpenBox();

	FORCEINLINE EFuseBoxState GetFuseBoxState() const { return FuseBoxState; }
	void SetOwnerRoom(AActor* InRoom) { OwnerRoom = InRoom; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnFuseItem();

	UFUNCTION()
	void OnFuseItemDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_FuseBoxState();

	void TurnOffRoomLights();

	UFUNCTION()
	void OnRep_ShakeStep();

	void DisableShakeEffect();

	UPROPERTY(EditDefaultsOnly, Category = "Fuse")
	TSoftObjectPtr<UPGItemData> FuseItemDataPath;

	// Sound
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName CoverFallSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName CoverShakeSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound", meta = (AllowPrivateAccess = "true"))
	FName FuseTakeSound;

	// Cover Shake
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ShakeEffect", meta = (AllowPrivateAccess = "true"))
	FName ShakeParameterName = TEXT("WPOPower");

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UGameplayAbility> InteractAbility;

	UPROPERTY(VisibleAnywhere, Category = "Root")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> CoverMesh;

	// 퓨즈 스폰 위치
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<USceneComponent> FuseAttachPoint;

	UPROPERTY()
	TObjectPtr<APGItemActor> SpawnedFuseItem;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> OwnerRoom;

	// Cover Shake
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MIDCover;

	FTimerHandle ShakeEffectTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_FuseBoxState)
	EFuseBoxState FuseBoxState = EFuseBoxState::Closed;

	UPROPERTY(ReplicatedUsing = OnRep_ShakeStep)
	uint8 ShakeStep = 0;
};
