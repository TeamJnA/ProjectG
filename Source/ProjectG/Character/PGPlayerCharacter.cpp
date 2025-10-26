// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerController.h"

#include "Game/PGAdvancedFriendsGameInstance.h"

// Essential Character Components
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Gameplay Ability System Components
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"

// UI and Components
#include "Component/PGInventoryComponent.h"
#include "Item/PGItemData.h"
#include "Component/PGSoundManagerComponent.h"
#include "UI/PGHUD.h"
#include "UI/PGMessageManagerWidget.h"
#include "UI/PGInventoryWidget.h"
#include "UI/PGScoreBoardWidget.h"
#include "Components/SpotLightComponent.h"

// Interface
#include "Interface/InteractableActorInterface.h"
#include "Interact/Ability/GA_Interact_Revive.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APGPlayerCharacter::APGPlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a first person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->SetIsReplicated(true);

	//Attach ItemSocket on character
	//middle_metacarpal_r
	EquippedItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedItemMesh"));
	EquippedItemMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
	EquippedItemMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	HeadlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadlightMesh"));
	HeadlightMesh->SetupAttachment(FirstPersonCamera);
	HeadlightMesh->SetVisibility(true);
	HeadlightMesh->SetOwnerNoSee(true);

	HeadlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightLight"));
	HeadlightLight->SetupAttachment(FirstPersonCamera);
	HeadlightLight->SetVisibility(false);
	HeadlightLight->SetIsReplicated(true);

	// Create Components
	InventoryComponent = CreateDefaultSubobject<UPGInventoryComponent>(TEXT("InventoryComponent"));

	SoundManagerComponent = CreateDefaultSubobject<UPGSoundManagerComponent>(TEXT("SoundManagerComponent"));

	// Set hand actions anim montages
	HandActionMontageType = EHandActionMontageType::Pick;;

	// 0 : Pick Anim Montage
	static ConstructorHelpers::FObjectFinder<UAnimMontage> PickMontageRef(TEXT("/Game/ProjectG/Character/Animation/Interact/AM_Pick_Item.AM_Pick_Item"));
	if (PickMontageRef.Object){
		HandActionAnimMontages.Add(PickMontageRef.Object);
	}

	// 1 : Change Anim Montage
	static ConstructorHelpers::FObjectFinder<UAnimMontage> ChangeMontageRef(TEXT("/Game/ProjectG/Character/Animation/Interact/AM_Pick_Item.AM_Pick_Item"));
	if (ChangeMontageRef.Object){
		HandActionAnimMontages.Add(ChangeMontageRef.Object);
	}

	// 2 : Drop Anim Montage
	static ConstructorHelpers::FObjectFinder<UAnimMontage> DropMontageRef(TEXT("/Game/ProjectG/Character/Animation/Interact/AM_Pick_Item.AM_Pick_Item"));
	if (DropMontageRef.Object) {
		HandActionAnimMontages.Add(DropMontageRef.Object);
	}
}

void APGPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGPlayerCharacter, bIsRagdoll);
	DOREPLIFETIME(APGPlayerCharacter, DeadPlayerState);
}

void APGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//if (Controller->IsLocalController())
	//{
	//	UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::BeginPlay: Init HUD [%s] | HasAuthority %d"), *GetNameSafe(this), HasAuthority());

	//	InitHUD();
	//}
}

void APGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APGPlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APGPlayerCharacter::Look);

		//Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APGPlayerCharacter::StartInputActionByTag, SprintTag);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APGPlayerCharacter::StopInputActionByTag, SprintTag);

		//Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APGPlayerCharacter::StartInputActionByTag, CrouchTag);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APGPlayerCharacter::StopInputActionByTag, CrouchTag);

		//HeadLight
		EnhancedInputComponent->BindAction(HeadLightAction, ETriggerEvent::Started, this, &APGPlayerCharacter::ToggleHeadLight);
		
		//Interacting
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APGPlayerCharacter::AddTagToCharacter, InteractTag);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &APGPlayerCharacter::RemoveTagFromCharacter, InteractTag);

		//DropItem
		EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Started, this, &APGPlayerCharacter::DropItem);

		//Mouse Clicks
		EnhancedInputComponent->BindAction(MouseLeftAction, ETriggerEvent::Started, this, &APGPlayerCharacter::AddTagToCharacter, MouseLeftTag);
		EnhancedInputComponent->BindAction(MouseLeftAction, ETriggerEvent::Completed, this, &APGPlayerCharacter::RemoveTagFromCharacter, MouseLeftTag);
		
		EnhancedInputComponent->BindAction(MouseRightAction, ETriggerEvent::Started, this, &APGPlayerCharacter::AddTagToCharacter, MouseRightTag);
		EnhancedInputComponent->BindAction(MouseRightAction, ETriggerEvent::Completed, this, &APGPlayerCharacter::RemoveTagFromCharacter, MouseRightTag);

		//ChangeItemSlot
		UPGAdvancedFriendsGameInstance * PGAdvancedFriendsGameInstance = Cast<UPGAdvancedFriendsGameInstance>(GetGameInstance());

		int32 _MaxInventorySize;
		_MaxInventorySize = PGAdvancedFriendsGameInstance ? PGAdvancedFriendsGameInstance->GetMaxInventorySize() : 5;

		for (int32 i = 0; i < _MaxInventorySize ; ++i)
		{
			if(ChangeItemSlotAction[i])
				EnhancedInputComponent->BindAction(ChangeItemSlotAction[i], ETriggerEvent::Started, this, &APGPlayerCharacter::ChangingItemSlot, i);
		}
	}
	else
	{
		//UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

bool APGPlayerCharacter::IsValidAttackableTarget() const
{
	// Check player is valid by checking gameplay tag.
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead")))
	{
		return false;
	}
	return true;
}

void APGPlayerCharacter::OnAttacked(FVector InstigatorHeadLocation)
{
	// This function only performed on server.
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] OnAttacked"), *GetNameSafe(this));

	// Remove all abilities.
	AbilitySystemComponent->ClearAllAbilities();
	
	// Stop character movement.
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// Set character Rotation.
	const FRotator CurrentRotation = GetActorRotation();
	FRotator NewCharacterRotation = (InstigatorHeadLocation - GetActorLocation()).Rotation();
	NewCharacterRotation.Pitch = 0.0f;
	NewCharacterRotation.Roll = 0.0f;

	SetActorRotation(NewCharacterRotation);

	// Set character Location. 
	const float EnemyCharacterDistance = 180.0f;

	FVector NewCharacterLocation = GetActorLocation();
	NewCharacterLocation.Z = InstigatorHeadLocation.Z - 50;

	FVector EnemyToCharacterDirection = (NewCharacterLocation - InstigatorHeadLocation).GetSafeNormal();

	NewCharacterLocation = InstigatorHeadLocation + EnemyToCharacterDirection * EnemyCharacterDistance;

	SetActorLocation(NewCharacterLocation);

	// Notify client to replicate server-side attack handling
	Client_OnAttacked(GetActorLocation(), GetActorRotation());
}

void APGPlayerCharacter::Client_OnAttacked_Implementation(FVector NewLocation, FRotator NewRotation)
{
	// Stop player input.
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		DisableInput(PlayerController);
	}

	// Stop anim.
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// Set character location and rotation.
	SetActorLocation(NewLocation);
	SetActorRotation(NewRotation);
	Controller->SetControlRotation(NewRotation);

	// Make the character hidden to itself when attacked
	GetMesh()->SetOwnerNoSee(true);

	// TODO : Play attacked anim

}

// Called on the server when the enemy's attack has ended.
void APGPlayerCharacter::OnAttackFinished()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAttackFinished function is must be called on server."));
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead")))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s already dead, but OnAttacked."), *GetNameSafe(this));
		return;
	}

	// Add Player.State.Dead Tag to Server and Client. It makes player state dead.
	AbilitySystemComponent->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead"));
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead"));

	OnPlayerDeathAuthority();
}

void APGPlayerCharacter::OnDeadTagChanged(const FGameplayTag Tag, int32 TagCount)
{
	if (TagCount > 0)
	{
		OnPlayerDeathLocally();
	}
}

/*
* 플레이어 사망시 서버 처리
* PlayerList의 플레이어 상태를 사망 상태, 게임 종료 상태로 설정
* 플레이어 사망 후 모든 플레이어가 종료 상태인지 확인
* 종료상태인 경우 게임을 종료 상태로 변경
* 아직 플레이 중인 플레이어가 있는 경우 종료 상태인 플레이어들의 상태를 디스플레이하는 ScoreBoardWidget Init
*/
void APGPlayerCharacter::OnPlayerDeathAuthority()
{
	if (!HasAuthority())
	{
		return;
	}

	APGGameState* GS = GetWorld()->GetGameState<APGGameState>();
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (GS && PS)
	{
		DeadPlayerState = PS;
		PS->SetHasFinishedGame(true);
		PS->SetIsDead(true);

		if (GS->IsGameFinished())
		{
			GS->SetCurrentGameState(EGameState::EndGame);
			GS->NotifyGameFinished();
		}
		else
		{
			FTimerHandle ScoreBoardTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(ScoreBoardTimerHandle, this, &APGPlayerCharacter::Client_InitScoreBoardWidget, 2.0f, false);
		}
	}

	// Stop character movement and animation
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// TODO : 플레이어 아이템들 드랍 [ Server ]

	// Ragdoll character ( Server. Client ragdoll is on OnRep_IsRagdoll )
	bIsRagdoll = true;
	OnRep_IsRagdoll();
}

// This function is called on Client when [Player.State.Dead] tag was added.
// == When player dead.
void APGPlayerCharacter::OnPlayerDeathLocally()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	// 본인이 보이도록 
	GetMesh()->SetOwnerNoSee(false);

	// TODO : 물리고 나서 카메라 천천히 멀어지기 [ 나중구현 ] ( Client )

	FirstPersonCamera->Deactivate();

	FollowCamera->Activate();
	

	// 2. 관전 버튼을 통해 관전 기능 추가.( Client )
	//APGPlayerController* PGPC = Cast<APGPlayerController>(Controller);
	//if (PGPC)
	//{
	//	PGPC->StartSpectate();
	//}
}

// Make client character ragdoll.
void APGPlayerCharacter::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
		GetCapsuleComponent()->SetSimulatePhysics(true);

		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
		
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void APGPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	InitDefaultAttributes();
	GiveAndActivatePassiveEffects();

	if (IsLocallyControlled()) //
	{
		UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::PossessedBy: Init HUD [%s]"), *GetNameSafe(this)); //
		InitHUD(); //

		// Bind "Player.State.Dead" to handle player death when the  tag is applied.
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Player.State.Dead"));

		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &APGPlayerCharacter::OnDeadTagChanged);
	}
}

//This function is called on the [CLIENT] When the server updates PlayerState.
void APGPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilitySystemComponent();
	InitDefaultAttributes();
	
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::OnRep_PlayerState: Init HUD [%s]"), *GetNameSafe(this)); //
		InitHUD(); //

		// Bind "Player.State.Dead" to handle player death when the  tag is applied.
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Player.State.Dead"));

		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &APGPlayerCharacter::OnDeadTagChanged);
	}
}

void APGPlayerCharacter::OnMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void APGPlayerCharacter::InitAbilitySystemComponent()
{
	APGPlayerState* PGPlayerState = GetPlayerState<APGPlayerState>();
	if (!PGPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("There's no PGPlayerState!!!"));
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("In the Server!!!"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("In the Client!!!"));
		}
		return;
	}
	//check(PGPlayerState);
	AbilitySystemComponent = CastChecked<UPGAbilitySystemComponent>(
		PGPlayerState->GetAbilitySystemComponent());
	AbilitySystemComponent->InitAbilityActorInfo(PGPlayerState, this);
	AttributeSet = PGPlayerState->GetAttributeSet();
	
	//Bind attribute change delegate
	FOnGameplayAttributeValueChange& OnMovementSpeedChangedDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AttributeSet->GetMovementSpeedAttribute()
	);
	MovementSpeedChangedDelegateHandle = OnMovementSpeedChangedDelegate.AddUObject(this, &APGPlayerCharacter::OnMovementSpeedChanged);
}

/*
* Possess 이후 HUD에서 PC 접근 시 유효하지 않은 경우 발생
* -> HUD에서 위젯 생성 후 직접 자신의 정보 바인드
*/
void APGPlayerCharacter::InitHUD()
{
	const APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
		{
			HUD->Init();

			UPGMessageManagerWidget* MessageManager = HUD->GetMessageManagerWidget();
			if (MessageManager)
			{
				MessageManager->BindMessageEntry(this);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("APGPlayerCharacter::InitHUD: MessageManagerWidget is NULL in HUD!")); //
			}

			UPGInventoryWidget* InventoryWidget = HUD->GetInventoryWidget();
			if (InventoryWidget)
			{
				InventoryWidget->BindInventorySlots(this);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("APGPlayerCharacter::InitHUD: InventoryWidget is NULL in HUD!")); //
			}
		}
	}
}

/*
* 플레이어 탈출 시 목표 지점으로 이동
*/
void APGPlayerCharacter::StartAutomatedMovement(const FVector& TargetLocation)
{
	bIsMovingAutomated = true;
	AutomatedMoveTarget = TargetLocation;
	
	AbilitySystemComponent->ClearAllAbilities();

	GetWorld()->GetTimerManager().SetTimer(AutomatedMoveTimer, this, &APGPlayerCharacter::UpdateAutomatedMovement, GetWorld()->GetDeltaSeconds(), true);
}

/*
* 이동 구현부
*/
void APGPlayerCharacter::UpdateAutomatedMovement()
{
	if (!bIsMovingAutomated)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutomatedMoveTimer);
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	if (FVector::DistSquared2D(CurrentLocation, AutomatedMoveTarget) < FMath::Square(50.0f))
	{
		UE_LOG(LogTemp, Log, TEXT("Character::UpdateAutomatedMovement: End move"))
		bIsMovingAutomated = false;
		GetWorld()->GetTimerManager().ClearTimer(AutomatedMoveTimer);

		OnAutomatedMovementCompleted.Broadcast();
	}
	else
	{
		FVector DirectionToTarget = AutomatedMoveTarget - CurrentLocation;
		DirectionToTarget.Z = 0.0f;
		const FVector WorldDirectionToTarget = DirectionToTarget.GetSafeNormal();
		UE_LOG(LogTemp, Log, TEXT("Character::UpdateAutomatedMovement: moving"));

		AddMovementInput(WorldDirectionToTarget, 1.0f);
	}
}

TSubclassOf<UGameplayAbility> APGPlayerCharacter::GetAbilityToInteract() const
{
	if (bIsRagdoll)
	{
		return UGA_Interact_Revive::StaticClass();
	}
	return nullptr;
}

FInteractionInfo APGPlayerCharacter::GetInteractionInfo() const
{
	if (bIsRagdoll)
	{
		return FInteractionInfo(EInteractionType::Hold, 3.0f);
	}
	return FInteractionInfo();
}

bool APGPlayerCharacter::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (bIsRagdoll)
	{
		//if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ReviveKit"))))
		//{
		//	return true;
		//}
		//OutFailureMessage = FText::FromString(TEXT("Need Revive Kit"));
		//return false;

		return true; // for test
	}
	return false;
}

void APGPlayerCharacter::HighlightOn() const
{
	if (bIsRagdoll && GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(true);
	}
}

void APGPlayerCharacter::HighlightOff() const
{
	if (bIsRagdoll && GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
}

/*
* 실패 메시지 디스플레이
*/
void APGPlayerCharacter::Client_DisplayInteractionFailedMessage_Implementation(const FText& Message)
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
			{
				HUD->DisplayInteractionFailedMessage(Message, 1.0f);
			}
		}
	}
}

void APGPlayerCharacter::MC_SetHeadlightState(bool _bIsFlashlightOn)
{
	HeadlightLight->SetVisibility(_bIsFlashlightOn);
}

void APGPlayerCharacter::ToggleHeadLight()
{
	if (!HeadlightLight->GetVisibleFlag())
	{
		ActivateAbilityByTag(HeadLightTag);
	}
	else
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->CancelAbilities(&HeadLightTag);
		}
	}
}

/*
* 홀딩 진행률 디스플레이 위젯 업데이트
*/
void APGPlayerCharacter::Client_UpdateInteractionProgress_Implementation(float Progress)
{
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
			{
				HUD->UpdateInteractionProgress(Progress);
			}
		}
	}
}

/*
* HUD에 ScoreBoardWidget 생성 요청
*/
void APGPlayerCharacter::Client_InitScoreBoardWidget_Implementation()
{
	if (IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC)
		{
			if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
			{
				HUD->InitScoreBoardWidget();
			}
		}
	}
}

/*
* 플레이어가 바라보는 대상, 대상의 변화에 따른 대상 Highlight 처리, 메시지 위젯 처리
*/
void APGPlayerCharacter::Client_PlayerStareAtTarget_Implementation(AActor* TargetActor)
{
	// 1. 바라보는 대상이 이전과 동일한 경우 -> return
	if (TargetActor == StaringTargetActor)
	{
		return;
	}

	// 2. 바라보는 대상이 이전과 다른 경우
	// 2-1. 바라보는 대상이 있었고, 현재 유효하면 Highlight off
	if (IsValid(StaringTargetActor))
	{
		if (IInteractableActorInterface* OldActorInterface = Cast<IInteractableActorInterface>(StaringTargetActor))
		{
			OldActorInterface->HighlightOff();
		}
	}
	// 2-2. 새로 바라보는 대상이 있고, 유효하면 Highlight on
	if (IsValid(TargetActor))
	{
		if (IInteractableActorInterface* NewActorInterface = Cast<IInteractableActorInterface>(TargetActor))
		{
			NewActorInterface->HighlightOn();
		}
	}

	// 현재 바라보는 대상 캐싱, 메시지 팝업
	StaringTargetActor = TargetActor;
	OnStareTargetUpdate.Broadcast(StaringTargetActor);
}

void APGPlayerCharacter::AddTagToCharacter_Implementation(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag)
{
	if (AbilitySystemComponent && InputActionAbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Add tag to character : %s"), *InputActionAbilityTag.ToString());
		AbilitySystemComponent->AddReplicatedLooseGameplayTags(InputActionAbilityTag);
		AbilitySystemComponent->AddLooseGameplayTags(InputActionAbilityTag);
	}
}

void APGPlayerCharacter::RemoveTagFromCharacter_Implementation(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag)
{
	if (AbilitySystemComponent && InputActionAbilityTag.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Remove tag from character : %s"), *InputActionAbilityTag.ToString());
		AbilitySystemComponent->RemoveReplicatedLooseGameplayTags(InputActionAbilityTag);
		AbilitySystemComponent->RemoveLooseGameplayTags(InputActionAbilityTag);
	}
}

void APGPlayerCharacter::ActivateAbilityByTag(const FGameplayTagContainer Tag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(Tag, true);
	}
}

void APGPlayerCharacter::CacheInteractionTarget(AActor* CacheInteractTarget)
{
	InteractionTargetActor = CacheInteractTarget;
}

TObjectPtr<UAnimMontage> APGPlayerCharacter::GetHandActionAnimMontages()
{
	const int32 Index = static_cast<int32>(HandActionMontageType);

	if (HandActionAnimMontages.IsValidIndex(Index))
	{
		return HandActionAnimMontages[Index];
	}
	UE_LOG(LogTemp, Warning, TEXT("There's no anim montage in HandActionAnimMontages."));
	return nullptr;
}

void APGPlayerCharacter::SetHandActionAnimMontage(EHandActionMontageType _HandActionMontageType)
{
	// EHandActionMontageType 
	// 0 : Pick		1 : Change 	 2 : Drop
	HandActionMontageType = _HandActionMontageType;
}

void APGPlayerCharacter::PlayHandActionAnimMontage(EHandActionMontageType _HandActionMontageType)
{
	SetHandActionAnimMontage(_HandActionMontageType);

	FGameplayTag HandActionTag = FGameplayTag::RequestGameplayTag(FName("Gameplay.Ability.HandAction"));

	FGameplayTagContainer HandActionTagContainer;
	HandActionTagContainer.AddTag(HandActionTag);

	ActivateAbilityByTag(HandActionTagContainer);
}

// After HandAction, this function is called to activate currentitem's ability.
void APGPlayerCharacter::EquipCurrentInventoryItem()
{
	if (InventoryComponent->HasCurrentItem())
	{
		InventoryComponent->ActivateCurrentItemAbility();
		AttachMeshOnHand();
	}
}

void APGPlayerCharacter::AttachMeshOnHand()
{
	// Attach mesh on hand and multicast

	UE_LOG(LogTemp, Log, TEXT("Attach Item On Hand"));
}

void APGPlayerCharacter::DetachMeshOnHand()
{
	UE_LOG(LogTemp, Log, TEXT("Detach Item On Hand"));
}

void APGPlayerCharacter::RemoveItemFromInventory()
{
	InventoryComponent->RemoveCurrentItem();
}

void APGPlayerCharacter::SetItemMesh(const bool bIsVisible)
{
	
	TObjectPtr<UPGItemData> ItemMeshToAttach = InventoryComponent->GetCurrentItemMesh();
	if (!ItemMeshToAttach)
	{
		EquippedItemMesh->SetStaticMesh(nullptr);
		EquippedItemMesh->SetRelativeTransform(FTransform::Identity);
		return;
	}
	EquippedItemMesh->SetRelativeTransform(ItemMeshToAttach->ItemSocketOffset);
	EquippedItemMesh->SetStaticMesh(ItemMeshToAttach->ItemMesh);
}

void APGPlayerCharacter::SetRightHandIK()
{
	InventoryComponent->Server_CheckHeldItemChanged();
}

void APGPlayerCharacter::DropItem_Implementation()
{
	//Cannot drop item during a HandAction.
	const FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionActivateTag))
	{
		UE_LOG(LogTemp, Log, TEXT("Cannot drop item during a HandAction."));
		return;
	}

	FVector DropItemLocation;
	FRotator DropItemRotation;
	GetActorEyesViewPoint(DropItemLocation, DropItemRotation);
	DropItemLocation += GetActorForwardVector() * 10 + GetActorUpVector() * (-30);

	InventoryComponent->DropCurrentItem(DropItemLocation, DropItemRotation);
}

void APGPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APGPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
		if (LookAxisVector.Y != 0)
		{
			FRotator NewRotation;
			NewRotation = FirstPersonCamera->GetComponentRotation();
			Server_SendCameraRotation(NewRotation);
		}
	}
}

void APGPlayerCharacter::Server_SendCameraRotation_Implementation(FRotator NewRotation)
{
	FirstPersonCamera->SetWorldRotation(NewRotation);
}

//Activate Input Action Ability by Tag
void APGPlayerCharacter::StartInputActionByTag(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag)
{
	ActivateAbilityByTag(InputActionAbilityTag);
	/*if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(InputActionAbilityTag, true);
	}*/
}

//Cancel Input Action Ability by Tag
void APGPlayerCharacter::StopInputActionByTag(const FInputActionValue& Value, FGameplayTagContainer InputActionAbilityTag)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&InputActionAbilityTag);
	}
}

void APGPlayerCharacter::ChangingItemSlot(const FInputActionValue& Value, int32 NumofSlot)
{
	InventoryComponent->ChangeCurrentInventoryIndex(NumofSlot);
}