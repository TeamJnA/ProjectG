// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerController.h"

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
#include "Component/PGSoundManagerComponent.h"
#include "UI/PGHUD.h"
#include "UI/PGMessageManagerWidget.h"
#include "UI/PGInventoryWidget.h"
#include "UI/PGScoreBoardWidget.h"
#include "Components/SpotLightComponent.h"

//Interface
#include "Interface/InteractableActorInterface.h"

#include "Kismet/GameplayStatics.h"

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
	ItemSocket = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSocket"));
	ItemSocket->SetupAttachment(GetMesh(), TEXT("middle_metacarpal_r"));
	ItemSocket->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	HeadlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadlightMesh"));
	HeadlightMesh->SetupAttachment(FirstPersonCamera);
	HeadlightMesh->SetVisibility(true);
	HeadlightMesh->SetOwnerNoSee(true);

	HeadlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightLight"));
	HeadlightLight->SetupAttachment(FirstPersonCamera);
	HeadlightLight->SetVisibility(false);
	HeadlightLight->SetIsReplicated(true);

	// Create InventoryComponent and Set changing Itemslot Input Action

	// Create Components
	InventoryComponent = CreateDefaultSubobject<UPGInventoryComponent>(TEXT("InventoryComponent"));

	SoundManagerComponent = CreateDefaultSubobject<UPGSoundManagerComponent>(TEXT("SoundManagerComponent"));

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
	// [NEW] Do this on controller
	/*
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	*/
}

void APGPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APGPlayerCharacter, bIsRagdoll);
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
	
	// 서버에서의 작업이 끝나고 client 작업 호출해야 겠다.

	//					서버 클라
	// 
	// 입력 차단           O
	// 어빌 제거  O
	// 이동 차단  O
	// 애님 중지           O
	// 캐릭 이동           O
	// 잡히는 모션               ???
	// 근데 애니메이션 중지 이후에 어차피 잡히는 애니메이션 연출할거라 이건 좀 고민해야겠네.
	// Client의 사망 관련 처리는 OnRep을 이용해서 처리하는 것이 좋다.
	// 나 같은 경우는 Player.State.Dead의 추가를 기다리는 것이 어떨까.
	// 이후 처리 과정
	// 태그 추가  O      O
	// 아이템 드랍, 레그돌 [서버] / 카메라 멀어지기 후 관전 [클라] << Player Death 함수 ?

	// Set player dead and rotate camera to the enemy's head
	// And camera move away slowly, activate spectating mode
	UE_LOG(LogTemp, Log, TEXT("[%s] OnAttacked"), *GetNameSafe(this));

	// 1. Remove all abilities [ Server ]
	AbilitySystemComponent->ClearAllAbilities();
	
	// 2. 캐릭터 이동 차단
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 3 - 1. 캐릭터 회전 [ Server, Client ]
	const FRotator CurrentRotation = GetActorRotation();
	FRotator TargetRotation = (InstigatorHeadLocation - GetActorLocation()).Rotation();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	SetActorRotation(TargetRotation);

	UE_LOG(LogTemp, Log, TEXT("Location : [%s] Rotation : [%s] After SetActorRotation OnServer"), *GetActorRotation().ToString(), *GetActorLocation().ToString());

	// 3 - 2. 캐릭터를 몹 앞으로 이동. [ Server, Client ]
	float EnemyCharacterDistance = 150.0f;

	FVector NewCharacterLocation = GetActorLocation();
	NewCharacterLocation.Z = InstigatorHeadLocation.Z;

	FVector EnemyToCharacterDirection = (NewCharacterLocation - InstigatorHeadLocation).GetSafeNormal();

	NewCharacterLocation = InstigatorHeadLocation + EnemyToCharacterDirection * EnemyCharacterDistance;

	SetActorLocation(NewCharacterLocation);

	UE_LOG(LogTemp, Log, TEXT("Location : [%s] Rotation : [%s] After SetActorLocation OnServer"), *GetActorRotation().ToString(), *GetActorLocation().ToString());

	// Notify client to replicate server-side attack handling
	Client_OnAttacked(GetActorLocation(), GetActorRotation());
}

void APGPlayerCharacter::Client_OnAttacked_Implementation(FVector NewLocation, FRotator NewRotation)
{
	// 1. 플레이어 입력 차단 [ Client ]
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerController Test"));
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
		DisableInput(PlayerController);
	}

	// 2.  애니메이션 중단
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// 카메라 회전
	SetActorLocation(NewLocation);
	SetActorRotation(NewRotation);
	Controller->SetControlRotation(NewRotation);

	UE_LOG(LogTemp, Log, TEXT("Location : [%s] Rotation : [%s] After SetControlRotation OnClient"), *GetActorRotation().ToString(), *GetActorLocation().ToString());

	// 3. 본인이 안보이도록
	GetMesh()->SetOwnerNoSee(true);

	// 4. 잡히는 모션 진행 [ 이건 서버냐 클라냐 그것이 문제로다... ]

}

void APGPlayerCharacter::OnAttackFinished()
{
	// 적의 공격이 끝났을 때, 서버에서 처리되는 함수.
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

	// 1. Add Player.State.Dead Tag to Server and Client. It makes player state dead.
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

void APGPlayerCharacter::OnPlayerDeathAuthority()
{
	if (!HasAuthority())
	{
		return;
	}

	// 1. Stop character movement and animation
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	// 2. 플레이어 아이템들 드랍 [ Server ]

	// 3. Ragdoll character ( Server. Client ragdoll is on OnRep_IsRagdoll )
	bIsRagdoll = true;

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCapsuleComponent()->SetSimulatePhysics(true);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
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

	// 1. 물리고 나서 카메라 천천히 멀어지기 [ 나중구현 ] ( Client )

	/*
	FirstPersonCamera->Deactivate();

	FollowCamera->Activate();
	*/

	// 2. 관전 버튼을 통해 관전 기능 추가.( Client )
	APGPlayerController* PGPC = Cast<APGPlayerController>(Controller);
	if (PGPC)
	{
		PGPC->StartSpectate();
	}
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
	}
}

void APGPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilitySystemComponent();
	GiveDefaultAbilities();
	InitDefaultAttributes();
	GiveAndActivatePassiveEffects();

	if (NewController && NewController->IsLocalController()) //
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
	
	if (Controller && Controller->IsLocalController())
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
				// 현재 캐릭터(this)를 직접 전달하여 바인딩
				MessageManager->BindMessageEntry(this);
				UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::InitHUD: MessageManagerWidget Bound to character.")); //
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("APGPlayerCharacter::InitHUD: MessageManagerWidget is NULL in HUD!")); //
			}

			UPGInventoryWidget* InventoryWidget = HUD->GetInventoryWidget();
			if (InventoryWidget)
			{
				InventoryWidget->BindInventorySlots(this);
				UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::InitHUD: InventoryWidget Bound to character.")); //
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("APGPlayerCharacter::InitHUD: InventoryWidget is NULL in HUD!")); //
			}
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

void APGPlayerCharacter::Client_DisplayInteractionFailedMessage_Implementation(const FText& Message)
{
	if (Controller && Controller->IsLocalController())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
			{
				UE_LOG(LogTemp, Log, TEXT("Character::Client_DisplayInteractionFailedMessage: Interaction failed because door is locked"));
				HUD->DisplayInteractionFailedMessage(Message, 1.0f);
			}
		}
	}
}

void APGPlayerCharacter::MC_SetFlashlightState(bool _bIsFlashlightOn)
{
	HeadlightLight->SetVisibility(_bIsFlashlightOn);
}

void APGPlayerCharacter::Client_UpdateInteractionProgress_Implementation(float Progress)
{
	if (Controller && Controller->IsLocalController())
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

void APGPlayerCharacter::Client_InitScoreBoardWidget_Implementation()
{
	if (Controller && Controller->IsLocalController())
	{
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC)
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeUIOnly());

			if (APGHUD* HUD = Cast<APGHUD>(PC->GetHUD()))
			{
				HUD->InitScoreBoardWidget();

				UPGScoreBoardWidget* ScoreBoardWidget = HUD->GetScoreBoardWidget();
				if (ScoreBoardWidget)
				{
					ScoreBoardWidget->BindPlayerEntry(PC);
					UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::Client_InitScoreBoardWidget: ScoreBoardWidget Bound to character.")); //
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("APGPlayerCharacter::Client_InitScoreBoardWidget: ScoreBoardWidget is NULL in HUD!")); //
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PlayerCharacter::Client_InitScoreBoardWidget: Get HUD failed | HasAuthority = %d"), HasAuthority());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerCharacter::Client_InitScoreBoardWidget: Get player controller failed | HasAuthority = %d"), HasAuthority());
		}
	}
}

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