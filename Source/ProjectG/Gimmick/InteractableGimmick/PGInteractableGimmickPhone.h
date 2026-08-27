// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gimmick/InteractableGimmick/PGInteractableGimmickBase.h"
#include "PGInteractableGimmickPhone.generated.h"

#define LOCTEXT_NAMESPACE "PGInteraction"

class USphereComponent;
class APGPlayerCharacter;
class APGPlayerState;

UENUM(BlueprintType)
enum class EPGPhoneState : uint8
{
	Idle,
	Ringing,
	Disabled
};

/**
 * 
 */
UCLASS()
class PROJECTG_API APGInteractableGimmickPhone : public APGInteractableGimmickBase
{
	GENERATED_BODY()
	
public:
	APGInteractableGimmickPhone();

	virtual void GimmickInteract(AActor* Investigator) override;

	// IInteractableActorInterface~
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	// ~IInteractableActorInterface

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnEnterSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsValidPhoneTarget(const APGPlayerState* PS) const;
	APGPlayerState* GetValidPlayerState(AActor* OtherActor) const;

	void TryStartRinging();
	void StartRinging();
	void StopRinging();
	void SetPhoneState(EPGPhoneState NewState);

	UFUNCTION()
	void PlayRingSound();

	UFUNCTION()
	void OnRep_PhoneState();

	void ApplyPhoneVisualState();

	UFUNCTION()
	void OnRep_RingCount();

	void StopShake();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phone")
	TObjectPtr<UStaticMeshComponent> ReceiverMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MIDReceiver;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phone")
	TObjectPtr<USphereComponent> EnterSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phone")
	TObjectPtr<USphereComponent> ExitSphere;

	UPROPERTY(ReplicatedUsing = OnRep_PhoneState, VisibleAnywhere, BlueprintReadOnly, Category = "Phone")
	EPGPhoneState PhoneState = EPGPhoneState::Idle;

	UPROPERTY(EditAnywhere, Category = "Phone")
	float ActivationChance = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Phone")
	float RingStartDelay = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Phone")
	float RingInterval = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Phone")
	float InteractHoldDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Phone|Shake")
	float ShakePower = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Phone|Shake")
	float ShakeDuration = 0.5f;

	UPROPERTY(ReplicatedUsing = OnRep_RingCount)
	uint8 RingCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "UI Text")
	FText AnswerText = LOCTEXT("Phone_Off", "Off");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	FName RingSoundName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	FName HangUpSoundName;

	UPROPERTY(EditDefaultsOnly, Category = "Phone|Shake")
	FName ShakeParameterName = TEXT("WPOPower");

	/** 범위 안의 플레이어 (서버 전용) */
	TSet<TWeakObjectPtr<APGPlayerState>> PlayersInRange;

	/** 이번 방문에서 이미 확률 판정을 한 플레이어 (서버 전용) */
	TSet<TWeakObjectPtr<APGPlayerState>> RolledPlayers;

	FTimerHandle RingStartDelayHandle;
	FTimerHandle RingTimerHandle;
	FTimerHandle ShakeTimerHandle;
};

#undef LOCTEXT_NAMESPACE
