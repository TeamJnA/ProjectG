// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"

#include "Interface/InteractableActorInterface.h"

#include "PGSearchableSlotBase.generated.h"

class UArrowComponent;
class UGameplayAbility;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class ESlotInteractType : uint8
{
	Draw    UMETA(DisplayName = "Draw (DrawerType : SpawnedItem also move)"),
	Open    UMETA(DisplayName = "Open (DoorType : Item Location is fixed. Open both doors)")
};

UCLASS()
class PROJECTG_API APGSearchableSlotBase : public AActor, public IInteractableActorInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APGSearchableSlotBase();

	FORCEINLINE void SetSlotInteractionType(ESlotInteractType _SlotInteractionType)  { SlotInteractionType = _SlotInteractionType; }

	FORCEINLINE TObjectPtr<USceneComponent> GetItemSpawnPoint() const { return ItemSpawnPoint; }

	void InteractSlot();

	// 생성된 아이템을 Slot에 붙인다. 
	void AttachSpawnedItem(AActor* Item);

	// IInteractableActorInterface~
	virtual TSubclassOf<UGameplayAbility> GetAbilityToInteract() const override;
	virtual void HighlightOn() const override;
	virtual void HighlightOff() const override;
	virtual FInteractionInfo GetInteractionInfo() const override;
	virtual FText GetInteractionText() const override;
	virtual bool CanStartInteraction(UAbilitySystemComponent* InteractingASC, FInteractionPromptInfo& OutFailurePrompt) const override;
	// ~IInteractableActorInterface

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	FName SlotName;

	/*
	* Slot을 Draw or Open으로 타입을 나누고,
	* Draw면 아이템도 같이 이동하도록, Open이면 문 메쉬 두 개를 열도록.
	* 
	Draw 할 때, 아이템 액터의 로케이션 변경은 알아서 레플리케이트됨....(Attach 했을 경우)
	그러면 내부에 아이템 액터를 생성할 때, SetReplicateMovement(false)를 False로 하기.

	멀티플레이 고려 사항
	1. 아이템 Spawn 및 아이템을 Slot에 Attach : 서버에서만 해도 클라에 레플리케이션 됨.
	2. Draw의 경우. 일단 이대로 진행해도 될듯.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	ESlotInteractType SlotInteractionType = ESlotInteractType::Draw;

	// 서랍/문 메쉬. 1을 Root로 쓰자.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SlotMesh1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SlotMesh2;

	// 아이템이 스폰될 위치
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> ItemSpawnPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable")
	TObjectPtr<UArrowComponent> OpenDirectionArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "InteractAbility")
	TSubclassOf<UGameplayAbility> InteractAbility;

	///
	///	Timeline
	/// 
	
	// 타임라인 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Searchable|Timeline")
	TObjectPtr<UTimelineComponent> MovementTimeline;

	// 블루프린트에서 할당해 줄 Float 커브 (0.0에서 1.0으로 변하는 커브)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Timeline")
	TObjectPtr<UCurveFloat> DrawCurve;

	// 서랍이 열렸는지 상태를 저장하는 리플리케이트 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsDrawn, BlueprintReadOnly, Category = "Searchable|State")
	bool bIsDrawn = false;

	// bIsDrawn이 변경될 때 클라이언트에서 호출될 함수
	UFUNCTION()
	void OnRep_IsDrawn();

	// 타임라인의 진행 상황에 따라 호출될 업데이트 함수
	UFUNCTION()
	void UpdateTimeline(float Value);

	// 월드 기준 이동을 위한 변수
	FVector InitialLocation;
	FVector TargetLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Length")
	float DrawLength;
};
