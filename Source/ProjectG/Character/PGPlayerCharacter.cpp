// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGPlayerCharacter.h"
#include "Player/PGPlayerController.h"
#include "Player/PGLobbyPlayerController.h"

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
#include "UI/Manager/PGHUD.h"
#include "UI/HUD/PGMessageManagerWidget.h"
#include "UI/HUD/PGInventoryWidget.h"
#include "UI/PlayerEntry/ScoreBoard/PGScoreBoardWidget.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Character/Component/PGVOIPTalker.h"

// Interface
#include "Interface/InteractableActorInterface.h"
#include "Interface/CharacterAnimationInterface.h"
#include "Interact/Ability/GA_Interact_Revive.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Game Setting
#include "Player/PGGameUserSettings.h"
#include "Utils/PGVoiceUtils.h"
#include "Perception/AISense_Hearing.h"


APGPlayerCharacter::APGPlayerCharacter()
{
	bAlwaysRelevant = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(47.0f, 96.0f);

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

	HitCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("EnemyOverlapCapsule"));
	HitCapsule->SetupAttachment(RootComponent);
	HitCapsule->InitCapsuleSize(40.0f, 96.0f);
	HitCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitCapsule->SetCollisionObjectType(ECC_Pawn);
	HitCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	HitCapsule->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);
	HitCapsule->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);

	//Attach ItemSocket on character
	//middle_metacarpal_r
	EquippedItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedItemMesh"));
	EquippedItemMesh->SetupAttachment(GetMesh(), TEXT("hand_r"));
	EquippedItemMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	HeadlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadlightMesh"));
	HeadlightMesh->SetupAttachment(FirstPersonCamera);
	HeadlightMesh->SetVisibility(true);
	HeadlightMesh->SetOwnerNoSee(true);
	HeadlightMesh->bCastHiddenShadow = false;

	HeadlightLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("HeadlightLight"));
	HeadlightLight->SetupAttachment(FirstPersonCamera);
	HeadlightLight->SetVisibility(false);
	HeadlightLight->SetIsReplicated(true);

	HeadlightLight->SetIndirectLightingIntensity(0.0f);
	HeadlightLight->SetVolumetricScatteringIntensity(5.0f);

	// Create Components
	InventoryComponent = CreateDefaultSubobject<UPGInventoryComponent>(TEXT("InventoryComponent"));

	SoundManagerComponent = CreateDefaultSubobject<UPGSoundManagerComponent>(TEXT("SoundManagerComponent"));

	StaminaBreathAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BreathAudioComponent"));
	StaminaBreathAudioComponent->bAutoActivate = false;

	HeartBeatAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HeartBeatAudioComponent"));
	HeartBeatAudioComponent->bAutoActivate = false;

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
	DOREPLIFETIME(APGPlayerCharacter, bIsTalking);
	DOREPLIFETIME(APGPlayerCharacter, FlickerLevel);
}

FGenericTeamId APGPlayerCharacter::GetGenericTeamId() const
{
	return FGenericTeamId((uint8)EGameTeam::Player);
}

void APGPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Set camera max pitch rotation
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->ViewPitchMax = 75.0f;
		PC->PlayerCameraManager->ViewPitchMin = -75.0f;
	}
}

void APGPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

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

		// debug, decrease sanity
		EnhancedInputComponent->BindAction(DebugDecreaseSanityAction, ETriggerEvent::Started, this, &APGPlayerCharacter::OnDebugDecreaseSanity);
	}
	else
	{
		//UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

bool APGPlayerCharacter::IsValidAttackableTarget() const
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player::IsValidAttackableTarget: no ASC"));
		return false;
	}
	// Check player is valid by checking gameplay tag.
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead"))
		|| AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.OnAttacked")))
	{
		return false;
	}
	return true;
}

void APGPlayerCharacter::OnAttacked(FVector InstigatorHeadLocation, const float EnemyCharacterDistance)
{
	// This function only performed on server.
	if (!HasAuthority())
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] OnAttacked"), *GetNameSafe(this));

	FGameplayTag AttackedTag = FGameplayTag::RequestGameplayTag("Player.State.OnAttacked");
	AbilitySystemComponent->AddLooseGameplayTag(AttackedTag);
	AbilitySystemComponent->AddReplicatedLooseGameplayTag(AttackedTag);

	// Remove all abilities.
	AbilitySystemComponent->ClearAllAbilities();
	ClearPassiveEffects();

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
	FVector NewCharacterLocation = GetActorLocation();
	NewCharacterLocation.Z = InstigatorHeadLocation.Z - 50;

	FVector EnemyToCharacterDirection = (NewCharacterLocation - InstigatorHeadLocation).GetSafeNormal();

	NewCharacterLocation = InstigatorHeadLocation + EnemyToCharacterDirection * EnemyCharacterDistance;

	SetActorLocation(NewCharacterLocation);

	// Notify client to replicate server-side attack handling
	Client_OnAttacked(GetActorLocation(), GetActorRotation());

	// 3초 후에 자동으로 OnAttackFinished(사망 처리)가 호출되도록 안전장치 설정
	// Enemy가 OnAttackFinished를 호출하지 않더라도 3초 뒤엔 확정 사망
	GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, this, &APGPlayerCharacter::OnAttackFinished, 4.0f, false);
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


	// Stop Audio
	if (HeartBeatAudioComponent)
	{
		HeartBeatAudioComponent->Stop();
	}
}

// Called on the server when the enemy's attack has ended.
void APGPlayerCharacter::OnAttackFinished()
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAttackFinished function is must be called on server."));
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(DeathTimerHandle);

	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Player.State.Dead")))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s already dead, but OnAttacked."), *GetNameSafe(this));
		return;
	}

	FGameplayTag AttackedTag = FGameplayTag::RequestGameplayTag("Player.State.OnAttacked");
	AbilitySystemComponent->RemoveLooseGameplayTag(AttackedTag);
	AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(AttackedTag);

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

	//플레이어 아이템들 드랍 [ Server ]
	InventoryComponent->DropAllItems(GetActorLocation());

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

	StopVoiceCheck();

	// 본인이 보이도록 
	GetMesh()->SetOwnerNoSee(false);

	// TODO : 물리고 나서 카메라 천천히 멀어지기 [ 나중구현 ] ( Client )
	FirstPersonCamera->Deactivate();
	FollowCamera->Activate();
}

// Make client character ragdoll.
void APGPlayerCharacter::OnRep_IsRagdoll()
{
	if (bIsRagdoll)
	{
		HighlightOn();

		GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
		GetCapsuleComponent()->SetSimulatePhysics(true);

		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
		
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		SetItemMesh(false);
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
		if (APGPlayerController* PC = Cast<APGPlayerController>(NewController))
		{
			PC->RefreshVoiceChannel();

			// When spawned first time, wait input for play level sequence
			if (PC->bGameStartFirstSpawned)
			{
				UE_LOG(LogTemp, Log, TEXT("Server Player spawned first time. Stop input for play level sequence."));

				DisableInput(PC);

				PC->SetIgnoreMoveInput(true);
				PC->SetIgnoreLookInput(true);

#if WITH_EDITOR
				EnableInput(PC);
				PC->SetIgnoreMoveInput(false);
				PC->SetIgnoreLookInput(false);
#endif
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Server Player spawned not first. InitHUD."));

				InitHUD();
			}
		}
		else
		{
			InitHUD();
		}

		UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::PossessedBy: Init PostProcess [%s]"), *GetNameSafe(this)); //
		InitPostProcessMaterial();

		// Bind "Player.State.Dead" to handle player death when the  tag is applied.
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Player.State.Dead"));

		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &APGPlayerCharacter::OnDeadTagChanged);

		if (HeadlightLight)
		{
			HeadlightLight->SetIndirectLightingIntensity(1.0f);
			HeadlightLight->SetVolumetricScatteringIntensity(0.2f);
		}

		GetWorldTimerManager().SetTimer(VoiceCheckTimerHandle, this, &APGPlayerCharacter::CheckVoiceAndReportNoise, 0.2f, true);
	}

	TryInitVoiceSettings();
	TrySetDeadCharacter();
}

//This function is called on the [CLIENT] When the server updates PlayerState.
void APGPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (!GetPlayerState())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Character] OnRep_PlayerState: PS is not valid"));
		return;
	}

	InitAbilitySystemComponent();
	InitDefaultAttributes();

	if (IsLocallyControlled())
	{
		if (APGPlayerController* PC = Cast<APGPlayerController>(GetController()))
		{
			PC->RefreshVoiceChannel();

			// When spawned first time, wait input for play level sequence
			if (PC->bGameStartFirstSpawned)
			{
				UE_LOG(LogTemp, Log, TEXT("Client Player spawned first time. Stop input for play level sequence."));

				DisableInput(PC);

				PC->SetIgnoreMoveInput(true);
				PC->SetIgnoreLookInput(true);

#if WITH_EDITOR
				EnableInput(PC);
				PC->SetIgnoreMoveInput(false);
				PC->SetIgnoreLookInput(false);
#endif
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Client Player spawned not first. InitHUD."));

				InitHUD();
			}
		}
		else
		{
			InitHUD();
		}

		UE_LOG(LogTemp, Log, TEXT("APGPlayerCharacter::OnRep_PlayerState: Init PostProcess [%s]"), *GetNameSafe(this)); //
		InitPostProcessMaterial();

		// Bind "Player.State.Dead" to handle player death when the  tag is applied.
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(TEXT("Player.State.Dead"));

		AbilitySystemComponent->RegisterGameplayTagEvent(DeadTag, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &APGPlayerCharacter::OnDeadTagChanged);

		if (HeadlightLight)
		{
			HeadlightLight->SetIndirectLightingIntensity(1.0f);
			HeadlightLight->SetVolumetricScatteringIntensity(0.2f);
		}

		GetWorldTimerManager().SetTimer(VoiceCheckTimerHandle, this, &APGPlayerCharacter::CheckVoiceAndReportNoise, 0.2f, true);
	}

	TryInitVoiceSettings();
}

void APGPlayerCharacter::OnStaminaChanged(const FOnAttributeChangeData& Data)
{
	//TODO Meta Sound ... ?
	// StaminaExhaustedEffect 채우기.
	// Server : 0일 때, Apply Gameplay Effect
	// 중복 적용 방지 방안 생각하기....
	if (HasAuthority())
	{
		if (Data.NewValue <= 0.1f)
		{
			if (AbilitySystemComponent && !AbilitySystemComponent->HasMatchingGameplayTag(
				FGameplayTag::RequestGameplayTag(FName("Gameplay.State.Exhausted"))))
			{
				Server_ApplyGameplayEffectToSelf(StaminaExhaustedEffect);
			}
		}
	}
	
	if(IsLocallyControlled())
	{
		if (!StaminaBreathAudioComponent || !StaminaBreathAudioComponent->GetSound())
		{
			return;
		}

		if ((Data.NewValue <= 80.0f) && !StaminaBreathAudioComponent->IsPlaying())
		{
			StaminaBreathAudioComponent->SetFloatParameter(FName("InStamina"), Data.NewValue);
			// StaminaBreathAudioComponent->Activate();
			StaminaBreathAudioComponent->Play();
		}

		// Client : 60이하일 때, Apply Meta Sound
		if (Data.NewValue <= 0.1f && Data.OldValue >= 0.1f)
		{
			if (!HeartBeatAudioComponent || !HeartBeatAudioComponent->GetSound())
			{
				return;
			}
			HeartBeatAudioComponent->Play();
		}
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

	FOnGameplayAttributeValueChange& OnSanityChangedDelegate = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		AttributeSet->GetSanityAttribute()
	);
	SanityChangedDelegateHandle = OnSanityChangedDelegate.AddUObject(this, &APGPlayerCharacter::OnSanityChanged);

	UpdateSanityPostProcessEffect(AttributeSet->GetSanity(), AttributeSet->GetMaxSanity());

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UPGAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &APGPlayerCharacter::OnStaminaChanged);
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

void APGPlayerCharacter::OnEscapeFinished()
{
	SetActorHiddenInGame(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	HeadlightMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	EquippedItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	StopVoiceCheck();
}

void APGPlayerCharacter::RequestApplyGimmickEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
	Server_ApplyGameplayEffectToSelf(EffectClass);
}

void APGPlayerCharacter::Server_ApplyGameplayEffectToSelf_Implementation(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!AbilitySystemComponent || !EffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddInstigator(this, this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

float APGPlayerCharacter::GetSanityValue() const
{
	if (!AttributeSet)
	{
		return 100.0f;
	}
	return AttributeSet->GetSanity();
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
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.ReviveKit"))))
		{
			return true;
		}
		OutFailureMessage = FText::FromString(TEXT("Need Revive Kit"));
		return false;
	}
	return false;
}

void APGPlayerCharacter::HighlightOn() const
{
	if (bIsRagdoll && GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(true);
		GetMesh()->SetCustomDepthStencilValue(2);
	}
}

void APGPlayerCharacter::HighlightOff() const
{
	/*
	if (bIsRagdoll && GetMesh())
	{
		GetMesh()->SetRenderCustomDepth(false);
	}
	*/
}

void APGPlayerCharacter::OnRevive()
{
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Error, TEXT("Character::OnRevive: No valid PS"));
		return;
	}

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Character::OnRevive: No valid ASC"));
		return;
	}	

	const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Player.State.Dead"));
	if (!AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character::OnRevive: No dead tag"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("Character::OnRevive: %s - Remove Dead tag."), *GetName());
	AbilitySystemComponent->RemoveReplicatedLooseGameplayTag(DeadTag);
	AbilitySystemComponent->RemoveLooseGameplayTag(DeadTag);
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

void APGPlayerCharacter::SetHeadlightVisible(bool bVisible)
{
	HeadlightLight->SetVisibility(bVisible);
}

void APGPlayerCharacter::ToggleHeadLight()
{
	if (IsHeadlightLocked())
	{
		return;
	}

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

void APGPlayerCharacter::ClearCharacterAbility()
{
	AbilitySystemComponent->ClearAllAbilities();
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
	
	TObjectPtr<UPGItemData> ItemDataToAttach = InventoryComponent->GetCurrentItemMesh();
	if (!ItemDataToAttach || !bIsVisible)
	{
		EquippedItemMesh->SetStaticMesh(nullptr);
		EquippedItemMesh->SetRelativeTransform(FTransform::Identity);
		if (ICharacterAnimationInterface* AnimInterface = Cast<ICharacterAnimationInterface>(GetMesh()->GetAnimInstance()))
		{
			AnimInterface->SetHandPose(EHandPoseType::Default);
		}
	}
	else
	{
		EquippedItemMesh->SetRelativeTransform(ItemDataToAttach->ItemSocketOffset);
		EquippedItemMesh->SetStaticMesh(ItemDataToAttach->ItemMesh);
		if (ICharacterAnimationInterface* AnimInterface = Cast<ICharacterAnimationInterface>(GetMesh()->GetAnimInstance()))
		{
			AnimInterface->SetHandPose(ItemDataToAttach->HandPoseType);
		}
	}
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
		float Sensitivity = 1.0f;
		if (UPGGameUserSettings* Settings = UPGGameUserSettings::GetPGGameUserSettings())
		{
			// 0~1 슬라이더 -> 0.2~2.0 배율
			Sensitivity = FMath::Lerp(0.2f, 2.0f, Settings->CameraSensitivity);
		}

		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X * Sensitivity);
		AddControllerPitchInput(LookAxisVector.Y * Sensitivity);
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

void APGPlayerCharacter::OnDebugDecreaseSanity(const FInputActionValue& Value)
{
	if (IsLocallyControlled())
	{
		Server_Debug_DecreaseSanity();
	}
}

void APGPlayerCharacter::Server_Debug_DecreaseSanity_Implementation()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	if (!SanityDecreaseEffect)
	{
		return;
	}

	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddInstigator(this, this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(SanityDecreaseEffect, 1.0f, ContextHandle);

	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		UE_LOG(LogTemp, Log, TEXT("Character::Server_Debug_DecreaseSanity: Applied Sanity Decrease Effect to %s"), *GetNameSafe(this));
	}
}

void APGPlayerCharacter::InitPostProcessMaterial()
{
	if (!SanityNoiseMaterialClass)
	{
		return;
	}

	SanityNoiseMID = UMaterialInstanceDynamic::Create(SanityNoiseMaterialClass, this);

	if (SanityNoiseMID)
	{
		if (FirstPersonCamera)
		{
			FirstPersonCamera->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, SanityNoiseMID));
		}

		if (FollowCamera)
		{
			FollowCamera->PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, SanityNoiseMID));
		}
	}
}

void APGPlayerCharacter::OnSanityChanged(const FOnAttributeChangeData& Data)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	const float CurrentSanity = Data.NewValue;
	float MaxSanity = 100.0f;
	if (AttributeSet)
	{
		MaxSanity = AttributeSet->GetMaxSanity();
	}

	UpdateSanityPostProcessEffect(CurrentSanity, MaxSanity);
}

void APGPlayerCharacter::UpdateSanityPostProcessEffect(float CurrentSanity, float MaxSanity)
{
	if (!SanityNoiseMID)
	{
		return;
	}

	if (MaxSanity <= 0.0f)
	{
		MaxSanity = 100.0f;
	}

	const float SanityRatio = FMath::Clamp(CurrentSanity / MaxSanity, 0.0f, 1.0f);
	const float InvertedRatio = 1.0f - SanityRatio;
	const float CurveExponent = 2.0f;
	const float CurvedIntensity = FMath::Pow(InvertedRatio, CurveExponent);
	BaseNoiseIntensity = CurvedIntensity * 0.2f;

	if (bIsGhostGlitching)
	{
		return;
	}

	// Threshold == 60.0f
	// Sanity가 Threshold보다 높은 경우 Glitch x / 일반 Noise는 존재
	// Glitching 중 Sanity 회복되어 Threshold보다 높아질 경우 Glitching 중단
	if (CurrentSanity > GlitchThresholdSanity)
	{
		GetWorld()->GetTimerManager().ClearTimer(GlitchIntervalTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(GlitchDurationTimerHandle);
		bIsGlitching = false;

		SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), BaseNoiseIntensity);
		return;
	}

	// Glitching 중이라면 NoiseIntensity 수정 x (Glitch용 NoiseIntensity 사용)
	if (bIsGlitching)
	{
		return;
	}
	
	// 일반상태
	SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), BaseNoiseIntensity);
	
	// Glitching 타이머 안도는 경우에만 Scheduling
	if (!GlitchIntervalTimerHandle.IsValid())
	{
		ScheduleNextGlitch();
	}
}

void APGPlayerCharacter::ScheduleNextGlitch()
{
	float CurrentSanity = 100.0f;
	if (AttributeSet)
	{
		CurrentSanity = AttributeSet->GetSanity();
	}

	if (CurrentSanity > GlitchThresholdSanity)
	{
		return;
	}

	float Interval = FMath::GetMappedRangeValueClamped(
		FVector2D(0.0f, GlitchThresholdSanity),
		FVector2D(3.0f, 10.0f),
		CurrentSanity
	);
	Interval += FMath::RandRange(-0.5f, 0.5f);
	if (Interval < 0.1f)
	{
		Interval = 0.1f;
	}

	GetWorld()->GetTimerManager().SetTimer(GlitchIntervalTimerHandle, this, &APGPlayerCharacter::StartGlitch, Interval, false);
}

void APGPlayerCharacter::StartGlitch()
{
	if (!SanityNoiseMID) 
	{
		return;
	}

	// GhostGlitching 중 일반 Glitch 차단
	if (bIsGhostGlitching)
	{
		return;
	}

	bIsGlitching = true;

	const float GlitchIntensity = FMath::RandRange(0.8f, 1.0f);
	SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), GlitchIntensity);

	// Glitch sound?

	const float Duration = FMath::RandRange(0.1f, 0.2f);
	GetWorld()->GetTimerManager().SetTimer(GlitchDurationTimerHandle, this, &APGPlayerCharacter::StopGlitch, Duration, false);
}

void APGPlayerCharacter::StopGlitch()
{
	if (!SanityNoiseMID) 
	{
		return;
	}

	// GhostGlitching 중 일반 Glitch 차단
	if (bIsGhostGlitching)
	{
		return;
	}

	bIsGlitching = false;

	SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), BaseNoiseIntensity);

	ScheduleNextGlitch();
}

// Ghost Glitch + Film Grain
void APGPlayerCharacter::Client_TriggerGhostGlitch_Implementation()
{
	if (!SanityNoiseMID)
	{
		return;
	}

	bIsGhostGlitching = true;
	bIsGlitching = true;

	GetWorld()->GetTimerManager().ClearTimer(GlitchIntervalTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(GlitchDurationTimerHandle);

	SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), 1.5f);

	if (FirstPersonCamera)
	{
		FirstPersonCamera->PostProcessSettings.bOverride_FilmGrainIntensity = true;
		FirstPersonCamera->PostProcessSettings.FilmGrainIntensity = 1.5f;
	}
	if (FollowCamera)
	{
		FollowCamera->PostProcessSettings.bOverride_FilmGrainIntensity = true;
		FollowCamera->PostProcessSettings.FilmGrainIntensity = 1.5f;
	}

	GetWorld()->GetTimerManager().SetTimer(GhostGlitchTimerHandle, this, &APGPlayerCharacter::StartGhostGlitchFadeOut, 1.5f, false);

	UE_LOG(LogTemp, Log, TEXT("PlayerCharacter::GhostGlitch"));
}

void APGPlayerCharacter::StartGhostGlitchFadeOut()
{
	CurrentGhostGlitchIntensity = 1.5f;
	GetWorld()->GetTimerManager().SetTimer(GhostGlitchTimerHandle, this, &APGPlayerCharacter::UpdateGhostGlitchFadeOut, 0.05f, true, 0.5f);
}

void APGPlayerCharacter::UpdateGhostGlitchFadeOut()
{
	if (!SanityNoiseMID)
	{
		GetWorld()->GetTimerManager().ClearTimer(GhostGlitchTimerHandle);
		return;
	}

	CurrentGhostGlitchIntensity -= 0.05f;
	if (CurrentGhostGlitchIntensity <= BaseNoiseIntensity)
	{
		GetWorld()->GetTimerManager().ClearTimer(GhostGlitchTimerHandle);
		bIsGhostGlitching = false;
		bIsGlitching = false;
		SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), BaseNoiseIntensity);

		if (FirstPersonCamera)
		{
			FirstPersonCamera->PostProcessSettings.bOverride_FilmGrainIntensity = false;
		}
		if (FollowCamera)
		{
			FollowCamera->PostProcessSettings.bOverride_FilmGrainIntensity = false;
		}

		if (AttributeSet)
		{
			UpdateSanityPostProcessEffect(AttributeSet->GetSanity(), AttributeSet->GetMaxSanity());
		}
	}
	else
	{
		const float GrainIntensity = FMath::Clamp(CurrentGhostGlitchIntensity, 0.2f, 1.5f);
		if (FirstPersonCamera)
		{
			FirstPersonCamera->PostProcessSettings.FilmGrainIntensity = GrainIntensity;
		}
		if (FollowCamera)
		{
			FollowCamera->PostProcessSettings.FilmGrainIntensity = GrainIntensity;
		}

		SanityNoiseMID->SetScalarParameterValue(FName("NoiseIntensity"), CurrentGhostGlitchIntensity);
	}
}

void APGPlayerCharacter::EnterGhostZone(AActor* Ghost)
{
	CachedGhost = Ghost;
	GetWorldTimerManager().SetTimer(GhostDistanceCheckTimerHandle, this, &APGPlayerCharacter::UpdateFlickerLevel, 0.2f, true);
}

void APGPlayerCharacter::ExitGhostZone()
{
	CachedGhost.Reset();
	GetWorldTimerManager().ClearTimer(GhostDistanceCheckTimerHandle);
	Server_UpdateGhostZoneEffect(0, 0.0f);
}

void APGPlayerCharacter::UpdateFlickerLevel()
{
	if (!CachedGhost.IsValid())
	{
		ExitGhostZone();
		return;
	}

	const float RawDistance = FVector::Dist(GetActorLocation(), CachedGhost->GetActorLocation());
	const float Distance = FMath::Max(RawDistance - 200.0f, 0.0f);

	uint8 NewLevel;
	if (Distance < GhostZoneNear)
	{
		NewLevel = 3;
	}
	else if (Distance < GhostZoneMedium)
	{
		NewLevel = 2;
	}
	else
	{
		NewLevel = 1;
	}

    // Level 변경 여부와 관계없이 매번 전달 (Intensity/ConeAngle 연속 보간)
	Server_UpdateGhostZoneEffect(NewLevel, Distance);
}

void APGPlayerCharacter::Server_UpdateGhostZoneEffect_Implementation(uint8 NewLevel, float DistanceToGhost)
{
	const uint8 OldLevel = FlickerLevel;
	FlickerLevel = NewLevel;

	// 범위 진입 시: 헤드라이트 꺼져있으면 강제로 켜기
	if (OldLevel == 0 && NewLevel > 0)
	{
		if (AbilitySystemComponent && !HeadlightLight->GetVisibleFlag())
		{
			ActivateAbilityByTag(HeadLightTag);
		}
	}

	// Level 변경 시 Flicker Effect 적용
	if (OldLevel != NewLevel)
	{
		ApplyFlickerEffect(NewLevel);
	}

	if (NewLevel == 0)
	{
		// 복원
		Multicast_SetHeadlightConeAngle(DefaultHeadlightConeAngle);
	}
	else
	{
		const float Alpha = FMath::Clamp(1.0f - (DistanceToGhost / 1300.0f), 0.0f, 1.0f);
		const float NewConeAngle = FMath::Lerp(DefaultHeadlightConeAngle, MinHeadlightConeAngle, Alpha);
		Multicast_SetHeadlightConeAngle(NewConeAngle);
	}
}

void APGPlayerCharacter::Multicast_SetHeadlightConeAngle_Implementation(float NewConeAngle)
{
	if (HeadlightLight)
	{
		HeadlightLight->SetOuterConeAngle(NewConeAngle);
		HeadlightLight->SetInnerConeAngle(NewConeAngle);
	}
}

void APGPlayerCharacter::ApplyFlickerEffect(uint8 NewLevel)
{
	if (!HeadlightLight)
	{
		return;
	}

	if (NewLevel == 0)
	{
		GetWorldTimerManager().ClearTimer(FlickerOffTimerHandle);
		GetWorldTimerManager().ClearTimer(FlickerOnTimerHandle);
		HeadlightLight->SetVisibility(true);
	}
	else
	{
		// 첫 플리커 바로 실행
		FlickerOff();
	}
}

void APGPlayerCharacter::FlickerOff()
{
	if (!HeadlightLight)
	{
		return;
	}

	HeadlightLight->SetVisibility(false);

	const float OffDuration = FMath::RandRange(0.1f, 0.15f);
	GetWorldTimerManager().SetTimer(FlickerOnTimerHandle, this, &APGPlayerCharacter::FlickerOn, OffDuration, false);
}

void APGPlayerCharacter::FlickerOn()
{
	if (!HeadlightLight || FlickerLevel == 0)
	{
		return;
	}

	HeadlightLight->SetVisibility(true);

	const float NextFlickerDelay = GetFlickerInterval();
	GetWorldTimerManager().SetTimer(FlickerOffTimerHandle, this, &APGPlayerCharacter::FlickerOff, NextFlickerDelay, false);
}

float APGPlayerCharacter::GetFlickerInterval() const
{
	switch (FlickerLevel)
	{
		case 1:
		{
			return FMath::RandRange(2.0f, 3.0f);    // 가끔
		}

		case 2: 
		{
			return FMath::RandRange(1.0f, 1.6f);     // 보통
		}

		case 3: 
		{
			return FMath::RandRange(0.1f, 0.3f);   // 자주
		}

		default: 
		{
			return 1.0f;
		}
	}
}

// 로컬이 리모트에 대해 Voip를 구현해야함
void APGPlayerCharacter::TryInitVoiceSettings()
{
	FString NetModeStr = (GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server");
	UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] TryInitVoiceSettings triggered!"), *NetModeStr);

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] LocalPC not found! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerCharacter::TryInitVoiceSettings, 0.1f, false);
		return;
	}

	APGPlayerController* InGamePC = Cast<APGPlayerController>(LocalPC);
	APGLobbyPlayerController* LobbyPC = Cast<APGLobbyPlayerController>(LocalPC);
	if (!InGamePC && !LobbyPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] Valid PC not found! Retrying in 0.1s..."), *NetModeStr);
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerCharacter::TryInitVoiceSettings, 0.1f, false);
		return;
	}

	// 로컬이면 Mute/Unmute 갱신
	// 리모트면 대상의 Voip, Mute/Unmute 갱신
	if (!IsLocallyControlled())
	{
		APGPlayerState* TargetPS = GetPlayerState<APGPlayerState>();
		if (TargetPS && TargetPS->IsInactive())
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] TargetPS is Inactive. Aborting voice setup."));
			return;
		}

		if (!TargetPS || !TargetPS->GetUniqueId().IsValid()) 
		{
			UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] [%s] TargetPS not found! Retrying in 0.1s..."), *NetModeStr);
			FTimerHandle RetryHandle;
			GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerCharacter::TryInitVoiceSettings, 0.1f, false);
			return;
		}

		if (!VoipTalker)
		{
			VoipTalker = UPGVOIPTalker::CreateTalkerForPlayer(TargetPS);
			if (!VoipTalker)
			{
				UE_LOG(LogTemp, Error, TEXT("[VoiceDebug] Failed to create Talker. Retrying..."));
				FTimerHandle RetryHandle;
				GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerCharacter::TryInitVoiceSettings, 0.1f, false);
				return;
			}

			UpdateVoipSettings();
		}
	}

	if (InGamePC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] RefreshVoiceChannel PGPC"));
		InGamePC->RefreshVoiceChannel();
	}
	else if (LobbyPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] RefreshVoiceChannel LobbyPC"));
		LobbyPC->RefreshVoiceChannel();
	}
}

void APGPlayerCharacter::UpdateVoipSettings()
{
	if (!VoipTalker)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UpdateVoipSettings] No valid VOIP."));
		return;
	}

	APGPlayerState* TargetPS = GetPlayerState<APGPlayerState>();
	if (!TargetPS || TargetPS->IsInactive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VoiceDebug] TargetPS is Inactive. Aborting voip update."));
		return;
	}

	VoipTalker->RegisterWithPlayerState(TargetPS);
	if (TargetPS->IsInGame())
	{
		VoipTalker->Settings.ComponentToAttachTo = GetRootComponent();
		VoipTalker->Settings.AttenuationSettings = VoiceAttenuationAsset;
		VoipTalker->Settings.SourceEffectChain = VoiceEffectAsset;
	}
	else
	{
		VoipTalker->Settings.ComponentToAttachTo = nullptr;
		VoipTalker->Settings.AttenuationSettings = nullptr;
		VoipTalker->Settings.SourceEffectChain = nullptr;
	}
}

void APGPlayerCharacter::TrySetDeadCharacter()
{
	APGPlayerState* PS = GetPlayerState<APGPlayerState>();
	if (PS && PS->IsInactive())
	{
		UE_LOG(LogTemp, Warning, TEXT("[TrySetDeadCharacter] PS is Inactive (Player Left). Aborting."));
		return;
	}

	if (!PS || !PS->GetUniqueId().IsValid())	
	{
		UE_LOG(LogTemp, Warning, TEXT("[TrySetDeadCharacter] PS not found! Retrying in 0.1s..."));
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, this, &APGPlayerCharacter::TrySetDeadCharacter, 0.1f, false);
		return;
	}

	PS->SetPlayerCharacter(this);
	UE_LOG(LogTemp, Log, TEXT("[TrySetDeadCharacter] SUCCESS: Set Dead Character to %s for PS(%s)"), *GetName(), *PS->GetPlayerName());
}

void APGPlayerCharacter::CheckVoiceAndReportNoise()
{
	CurrentVoiceAmplitude = PGVoiceUtils::GetCurrentAmplitude(GetWorld());
	if (CurrentVoiceAmplitude < 0.02f)
	{
		//UE_LOG(LogTemp, Log, TEXT("[Character]: Too low Amplitude: %.2f"), CurrentVoiceAmplitude);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Character]: Amplitude: %.2f"), CurrentVoiceAmplitude);
	Server_ReportVoiceNoise(CurrentVoiceAmplitude);
}

void APGPlayerCharacter::Server_ReportVoiceNoise_Implementation(float Amplitude)
{
	const float ClampedAmplitude = FMath::Clamp(Amplitude, 0.0f, 1.0f);
	// Amplitude → SoundLevel 매핑
	// 0.05~0.15 (속삭임)    → SoundLevel 2, Range 400 (800)
	// 0.15~0.35 (일반 대화) → SoundLevel 3, Range 600 (1800)
	// 0.35~ (큰 소리)   → SoundLevel 4, Range 800 (3200)
	float SoundLevel;
	float MaxRange;
	if (ClampedAmplitude < 0.05f)
	{
		SoundLevel = 2.0f;
		MaxRange = 400.0f;
	}
	else if (ClampedAmplitude < 0.2f)
	{
		SoundLevel = 3.0f;
		MaxRange = 600.0f;
	}
	else
	{
		SoundLevel = 4.0f;
		MaxRange = 800.0f;
	}

	UE_LOG(LogTemp, Log, TEXT("[Character_Server]: SoundLevel: %.1f | MaxRange: %.1f"), SoundLevel, MaxRange);

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		GetActorLocation(),
		SoundLevel,
		this,
		MaxRange,
		FName("VoiceNoise")
	);

	bIsTalking = true;
	GetWorldTimerManager().ClearTimer(VoiceMonitoringTimerHandle);
	GetWorldTimerManager().SetTimer(VoiceMonitoringTimerHandle, [this]()
	{
		bIsTalking = false;
	}, 0.5f, false);
}

void APGPlayerCharacter::StopVoiceCheck()
{
	GetWorldTimerManager().ClearTimer(VoiceCheckTimerHandle);
	bIsTalking = false;
}
