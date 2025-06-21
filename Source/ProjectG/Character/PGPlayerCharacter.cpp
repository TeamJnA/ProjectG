// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGPlayerCharacter.h"

// #include "Game/PGGameInstance.h"
#include "Game/PGAdvancedFriendsGameInstance.h"

//Essential Character Components
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

//Gameplay Ability System Components
#include "AbilitySystem/PGAbilitySystemComponent.h"
#include "AbilitySystem/PGAttributeSet.h"
#include "Player/PGPlayerState.h"

//UI and Components
#include "Component/PGInventoryComponent.h"
#include "UI/PGHUD.h"

//Interface
#include "Interface/InteractableActorInterface.h"

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

	//Attach ItemSocket on character
	//middle_metacarpal_r
	ItemSocket = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(GetMesh(), TEXT("middle_metacarpal_r"));
	ItemSocket->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	// Create InventoryComponent and Set changing Itemslot Input Action
	InventoryComponent = CreateDefaultSubobject<UPGInventoryComponent>(TEXT("InventoryComponent"));

	// Set hand actions anim montages
	// EHandActionMontageType
	// 0 : Pick		1 : Change
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

void APGPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::BeginPlay: Init HUD [%s] | HasAuthority %d"), *GetNameSafe(this), HasAuthority());

		InitHUD();
	}
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

void APGPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	InitDefaultAttributes();
	GiveAndActivatePassiveEffects();
}

//This function is called on the client When the server updates PlayerState.
void APGPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilitySystemComponent();
	InitDefaultAttributes();
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
	}
	check(PGPlayerState);
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

void APGPlayerCharacter::InitHUD() const
{
	const APlayerController* PC = Cast<APlayerController>(Controller);
	if (PC)
	{
		if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
		{
			HUD->Init();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerCharacter::InitHUD: Get HUD failed | HasAuthority = %d"), HasAuthority());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerCharacter::InitHUD: Get player controller failed | HasAuthority = %d"), HasAuthority());
	}
}

void APGPlayerCharacter::Client_PlayerStareAtTarget_Implementation(AActor* TargetActor)
{
	if (TargetActor)
	{
		if (TargetActor != StaringTargetActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("APGPlayerCharacter::Client_PlayerStareAtTarget: %s"), *TargetActor->GetName());
			StaringTargetActor = TargetActor;
			OnStareTargetUpdate.Broadcast(TargetActor);
			if (IInteractableActorInterface* InterfaceIneract = Cast<IInteractableActorInterface>(TargetActor))
			{
				InterfaceIneract->HighlightOn();
			}
		}		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("APGPlayerCharacter::Client_PlayerStareAtTarget: null"));
		OnStareTargetUpdate.Broadcast(nullptr);
		if (IInteractableActorInterface* InterfaceIneract = Cast<IInteractableActorInterface>(StaringTargetActor))
		{
			InterfaceIneract->HighlightOff();
		}
		StaringTargetActor = nullptr;
	}
	
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

void APGPlayerCharacter::ActivateAbilityByTag(FGameplayTagContainer Tag)
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
	int32 Index = static_cast<int32>(HandActionMontageType);

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

//After HandAction, this function is called to activate currentitem's ability.
void APGPlayerCharacter::EquipCurrentInventoryItem()
{
	InventoryComponent->ActivateCurrentItemAbility();
	AttachMeshOnHand();
}

void APGPlayerCharacter::AttachMeshOnHand()
{
	// Need to Multicast.
	// Different in client and server.
	// Client : Attach mesh on sub Hand.
	// Server : Attach mesh on main hand and multicast

	UE_LOG(LogTemp, Log, TEXT("Attach Item On Hand"));
	/*
	void AMyCharacter::AttachItemMeshToHand(UItemData* ItemData)
	{
		if (!ItemData || !ItemData->ItemMesh)
			return;
        
		HeldItemMeshComponent->SetStaticMesh(ItemData->ItemMesh);
		HeldItemMeshComponent->AttachToComponent(GetMesh(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			TEXT("HandSocket"));
		HeldItemMeshComponent->SetVisibility(true);
	}

	void AMyCharacter::DetachItemMesh()
	{
		HeldItemMeshComponent->SetVisibility(false);
		HeldItemMeshComponent->SetStaticMesh(nullptr);
	}
	*/
}

void APGPlayerCharacter::DetachMeshOnHand()
{
	UE_LOG(LogTemp, Log, TEXT("Detach Item On Hand"));
}

void APGPlayerCharacter::RemoveItemFromInventory()
{
	InventoryComponent->RemoveCurrentItem();
}

void APGPlayerCharacter::DropItem_Implementation()
{
	//Cannot drop item during a HandAction.
	FGameplayTag HandActionActivateTag = FGameplayTag::RequestGameplayTag(FName("Player.Hand.Locked"));

	if (AbilitySystemComponent->HasMatchingGameplayTag(HandActionActivateTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot drop item during a HandAction."));
		return;
	}

	FVector DropItemLocation;
	FRotator TempRotation;
	GetActorEyesViewPoint(DropItemLocation, TempRotation);
	DropItemLocation += GetActorForwardVector() * 10 + GetActorUpVector() * (-10);

	InventoryComponent->DropCurrentItem(DropItemLocation);
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
