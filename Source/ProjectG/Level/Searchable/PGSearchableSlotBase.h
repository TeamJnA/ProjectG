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

UENUM(BlueprintType)
enum class ESlotMeshType : uint8
{
	// 캐비닛
	Cabinet_Door   UMETA(DisplayName = "Cabinet Door"),

	// 드레서1
	Dresser1_Door_Left		UMETA(DisplayName = "Dresser1 Door Left"),
	Dresser1_Door_Right	UMETA(DisplayName = "Dresser1 Door Right"),
	Dresser1_Drawer			UMETA(DisplayName = "Dresser1 Drawer (Middle)"),

	// 드레서2
	Dresser2_Door		UMETA(DisplayName = "Dresser2 Door(Upper)"),
	Dresser2_Drawer			UMETA(DisplayName = "Dresser2 Drawer(Lower)"),

	// 드레서3
	Dresser3_Door_Upper		UMETA(DisplayName = "Dresser3 Door Upper"),
	Dresser3_Door_Lower	UMETA(DisplayName = "Dresser3 Door Lower"),
	Dresser3_Drawer			UMETA(DisplayName = "Dresser3 Drawer (Middle)"),

	// 선반
	Shelf_Drawer     UMETA(DisplayName = "Shelf Drawer"),

	// TableSmall
	TableSmall_Drawer     UMETA(DisplayName = "TableSmall Drawer")
};

USTRUCT(BlueprintType)
struct FSlotVisualData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> Material;
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

	void SetCurrentSlotMesh(ESlotMeshType _InSlotMesh);

	void InteractSlot();

	// 생성된 아이템을 Slot에 붙인다. 
	UFUNCTION(BlueprintCallable)
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

	// Slot의 Mesh 및 Material을 저장해두는 Map
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Mesh")
	TMap<ESlotMeshType, FSlotVisualData> SlotMeshMap;

	UPROPERTY(ReplicatedUsing = OnRep_SlotMesh, BlueprintReadOnly, Category = "Searchable|Mesh")
	ESlotMeshType CurrentSlotMesh;

	UFUNCTION()
	void OnRep_SlotMesh();

	/*
	* 
	*/
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Searchable")
	ESlotInteractType SlotInteractionType = ESlotInteractType::Draw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	// 서랍/문 메쉬. 1을 Root로 쓰자.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SlotMesh1;

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
	
	// Open / Draw에 따른 타임라인 바인딩 및 목표 위치/회전값 계산을 처리할 함수
	void SetupTimeline();

	// 타임라인 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Searchable|Timeline")
	TObjectPtr<UTimelineComponent> MovementTimeline;

	// Draw용 커브
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Timeline")
	TObjectPtr<UCurveFloat> DrawCurve;

	// Open용 커브
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Timeline")
	TObjectPtr<UCurveFloat> OpenCurve;

	// 서랍이 열렸는지 상태를 저장하는 리플리케이트 변수
	UPROPERTY(ReplicatedUsing = OnRep_IsDrawn, BlueprintReadOnly, Category = "Searchable|State")
	bool bIsDrawn = false;

	// bIsDrawn이 변경될 때 클라이언트에서 호출될 함수
	UFUNCTION()
	void OnRep_IsDrawn();

	// 타임라인의 진행 상황에 따라 호출될 업데이트 함수
	UFUNCTION()
	void UpdateDrawTimeline(float Value);

	UFUNCTION()
	void UpdateOpenTimeline(float Value);

	// Draw의 월드 기준 이동을 위한 변수
	FVector InitialLocation;
	FVector TargetLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Draw")
	float DrawLength;

	// Open용 변수
	FRotator InitialRotation;
	FRotator TargetRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Searchable|Open")
	float OpenAngle = 120.0f; 
};
