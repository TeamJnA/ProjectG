// Fill out your copyright notice in the Description page of Project Settings.


#include "Level/Misc/PGDoor1.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Physics/PGChaosCacheManager.h"
#include "GameFramework/GameModeBase.h"

#include "AbilitySystemComponent.h"
#include "Interact/Ability/GA_Interact_Door.h"

#include "Sound/PGSoundManager.h"
#include "Interface/SoundManagerInterface.h"

#include "GenericTeamAgentInterface.h"

#include "Net/UnrealNetwork.h"

// Sets default values
APGDoor1::APGDoor1()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshRef(TEXT("/Script/Engine.StaticMesh'/Game/Imports/SICKA_mansion/StaticMeshes/SM_DoorCarved.SM_DoorCarved'"));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	DoorHinge = CreateDefaultSubobject<USceneComponent>(TEXT("DoorHinge"));
	DoorHinge->SetupAttachment(Root);
	DoorHinge->SetRelativeLocation(FVector(0.0f, 82.0f, 0.0f));

	Mesh0 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh0"));
	Mesh0->SetupAttachment(DoorHinge);
	if (MeshRef.Object)
	{
		Mesh0->SetStaticMesh(MeshRef.Object);
	}
	Mesh0->SetRelativeLocation(FVector(11.0f, 0.0f, 7.0f));	
	Mesh0->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh0->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.985f));

	Mesh0->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	Mesh0->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel4, ECR_Ignore);

	// Door does not affect to NavMesh. AI ignore door.
	Mesh0->SetCanEverAffectNavigation(false);

	const FRotator ClosedRotation = FRotator::ZeroRotator;
	const FVector ClosedLocation = FVector(0.0f, 82.0f, 0.0f);
	ClosedTransform = FTransform(ClosedRotation, ClosedLocation);

	const FRotator OpenedRotation_A = FRotator(0.0f, 90.0f, 0.0f);
	const FRotator OpenedRotation_B = FRotator(0.0f, -90.0f, 0.0f);
	const FVector OpenedLocation = FVector(0.0f, 74.0f, 0.0f);
	OpenedTransform_A = FTransform(OpenedRotation_A, OpenedLocation);
	OpenedTransform_B = FTransform(OpenedRotation_B, OpenedLocation);

	InteractAbility = UGA_Interact_Door::StaticClass();

	DoorOpenSound = FName(TEXT("LEVEL_Door_Open"));
	DoorCloseSound = FName(TEXT("LEVEL_Door_Close"));
	DoorUnlockSound = FName(TEXT("LEVEL_Door_Unlock"));
	LockedDoorSound = FName(TEXT("LEVEL_Door_Locked"));

	ShakeParameterName = TEXT("WPOPower");

	bDoorBrokened = false;

	static ConstructorHelpers::FClassFinder<APGChaosCacheManager> CCM_Closed_BP(TEXT("/Game/ProjectG/Levels/Room/Misc/DoorDestruction/BP_CCM_DoorClosed.BP_CCM_DoorClosed_C"));
	if (CCM_Closed_BP.Succeeded())
	{
		BP_PG_CCMClosed = CCM_Closed_BP.Class;
	}

	static ConstructorHelpers::FClassFinder<APGChaosCacheManager> CCM_Opened_BP(TEXT("/Game/ProjectG/Levels/Room/Misc/DoorDestruction/BP_CCM_DoorOpened.BP_CCM_DoorOpened_C"));
	if (CCM_Opened_BP.Succeeded())
	{
		BP_PG_CCMOpened = CCM_Opened_BP.Class;
	}

	DoorOpenType = EDoorOpenType::Closed;
}

void APGDoor1::BeginPlay()
{
	Super::BeginPlay();

	MIDDoor = Mesh0->CreateDynamicMaterialInstance(0);

	FString NetModeStr = (GetNetMode() == NM_Client) ? TEXT("Client") :
		(GetNetMode() == NM_DedicatedServer) ? TEXT("DedicatedServer") :
		(GetNetMode() == NM_ListenServer) ? TEXT("ListenServer") : TEXT("Standalone");

	FString AuthStr = HasAuthority() ? TEXT("TRUE") : TEXT("FALSE");

	UE_LOG(LogTemp, Warning, TEXT("[DOOR_CHECK] Name: %s | NetMode: %s | HasAuthority: %s | Location: %s"),
		*GetName(),
		*NetModeStr,
		*AuthStr,
		*GetActorLocation().ToString());
}

void APGDoor1::SpawnDoor(UWorld* World, const FTransform& Transform, const FActorSpawnParameters& SpawnParams, bool InbIsLocked)
{
	APGDoor1* NewDoor = World->SpawnActor<APGDoor1>(StaticClass(), Transform, SpawnParams);

	if (NewDoor)
	{
		NewDoor->bIsLocked = InbIsLocked;

		// Spawn Chaos Cache Managers
		TSubclassOf<APGChaosCacheManager> CCMOpenToSpawn = GetDefault<APGDoor1>()->BP_PG_CCMOpened;

		APGChaosCacheManager* SpawnedOpenCCM = World->SpawnActor<APGChaosCacheManager>(CCMOpenToSpawn, Transform, SpawnParams);

		if (SpawnedOpenCCM)
		{
			UE_LOG(LogTemp, Log, TEXT("APGDoor1 Succesfully Spawned APGChaosCacheManager : OpenCCM"));
			NewDoor->CCMOpened = SpawnedOpenCCM;
		}

		TSubclassOf<APGChaosCacheManager> CCMCloseToSpawn = GetDefault<APGDoor1>()->BP_PG_CCMClosed;

		APGChaosCacheManager* SpawnedCloseCCM = World->SpawnActor<APGChaosCacheManager>(CCMCloseToSpawn, Transform, SpawnParams);

		if (SpawnedCloseCCM)
		{
			UE_LOG(LogTemp, Log, TEXT("APGDoor1 Succesfully Spawned APGChaosCacheManager : SpawnedCloseCCM"));
			NewDoor->CCMClosed = SpawnedCloseCCM;
		}
	}
}

TSubclassOf<UGameplayAbility> APGDoor1::GetAbilityToInteract() const
{
	return InteractAbility;
}

void APGDoor1::ToggleDoor(AActor* InteractInvestigator)
{
	BreakDoorByEnemy(InteractInvestigator);

	SetDoorState(!bIsOpen, InteractInvestigator);
}

void APGDoor1::SetDoorState(bool InbIsOpen, AActor* InteractInvestigator)
{
	// Check door open sound twice.
	// when enemy overlap door, door open called forcely.
	const bool bOpenTwice = (bIsOpen && InbIsOpen);

	// Player play sound(Door open sound) for both player and enemy. Enemy sound for only player.
	// Player ID : 0, Enemy ID : 1
	IGenericTeamAgentInterface* TeamCheckInterface = Cast<IGenericTeamAgentInterface>(InteractInvestigator);
	bool bIsPlayer = !(TeamCheckInterface->GetGenericTeamId().GetId());

	bIsOpen = InbIsOpen;
	if (bIsOpen)
	{
		// If door already opened, do not change transform by enemy
		if (bOpenTwice)
		{
			return;
		}

		Mesh0->SetCanEverAffectNavigation(true);

		if (bIsPlayer)
		{
			PlayDoorSound(DoorOpenSound);
		}
		else
		{
			PlayDoorSound(DoorOpenSound, false);
			bIsLocked = false;
		}

		if (InteractInvestigator)
		{
			const FVector DoorToCharacter = InteractInvestigator->GetActorLocation() - GetActorLocation();
			const FVector DoorForwardVector = GetActorForwardVector();
			const float DotProduct = FVector::DotProduct(DoorForwardVector, DoorToCharacter);

			if (DotProduct < 0.0f)
			{
				DesiredTransform = OpenedTransform_A;
				DoorOpenType = EDoorOpenType::Opened_A;
			}
			else
			{
				DesiredTransform = OpenedTransform_B;
				DoorOpenType = EDoorOpenType::Opened_B;
			}
		}
		else
		{
			DesiredTransform = OpenedTransform_A;
			DoorOpenType = EDoorOpenType::Opened_A;
		}
	}
	else
	{
		PlayDoorSound(DoorCloseSound);

		Mesh0->SetCanEverAffectNavigation(false);

		DesiredTransform = ClosedTransform;
		DoorOpenType = EDoorOpenType::Closed;

	}


	OnRep_DesiredTransform();
}

void APGDoor1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APGDoor1, bIsOpen);
	DOREPLIFETIME(APGDoor1, bIsLocked);
	DOREPLIFETIME(APGDoor1, DesiredTransform);
	DOREPLIFETIME(APGDoor1, CCMOpened);
	DOREPLIFETIME(APGDoor1, CCMClosed);
	DOREPLIFETIME(APGDoor1, DoorOpenType);
}

void APGDoor1::Multicast_ActivateShakeEffect_Implementation()
{
	ToggleShakeEffect(true);

	// 0.1초 후 DisableEffect 함수를 호출하도록 타이머 설정 (TimerHandle1 관리)
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("DisableShakeEffect"));

	GetWorldTimerManager().SetTimer(
		ShakeEffectTimerHandle,
		TimerDelegate,
		0.1f,
		false
	);
}

void APGDoor1::DisableShakeEffect()
{
	ToggleShakeEffect(false);

	GetWorldTimerManager().ClearTimer(ShakeEffectTimerHandle);
}

void APGDoor1::ToggleShakeEffect(bool bToggle)
{
	float TargetValue = bToggle ? 1.0f : 0.0f;

	if (MIDDoor)
	{
		UE_LOG(LogTemp, Log, TEXT("ToggleShakeEffect PGDoor1 %f"), TargetValue);

		MIDDoor->SetScalarParameterValue(ShakeParameterName, TargetValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PGDoor Cannot Find MIDDoor"));
	}
}

void APGDoor1::BreakDoorByEnemy(AActor* InteractInvestigator)
{
	if (!CCMOpened || !CCMClosed)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot find chaos cache manager in PGDoor1"));
		SetDoorState(true, InteractInvestigator);
		return;
	}

	// Set door hidden and un interactable
	bDoorBrokened = true;
	SetActorHiddenInGame(true);

	// Set ChaosDestruction Start Transform
	FTransform TargetDoorTransform = Mesh0->GetComponentTransform();

	CCMClosed->SetActorTransform(TargetDoorTransform);
	CCMOpened->SetActorTransform(TargetDoorTransform);
	
	if (DoorOpenType == EDoorOpenType::Closed)
	{
		const FVector DoorToCharacter = InteractInvestigator->GetActorLocation() - GetActorLocation();
		const FVector DoorForwardVector = GetActorForwardVector();
		const float DotProduct = FVector::DotProduct(DoorForwardVector, DoorToCharacter);

		if (DotProduct > 0.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("Set CCMClosed Transform Dot+"));
			// 정방향
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Set CCMClosed Transform Dot-"));

			// 1. 필요한 월드 값들을 가져옵니다.
			FTransform ActorW = GetActorTransform();            // 액터의 월드 트랜스폼 (중심)
			FTransform MeshW = Mesh0->GetComponentTransform();  // 메쉬의 현재 월드 트랜스폼 (오른쪽 아래 피벗)

			// 2. 액터 중심 기준의 '180도 회전 행렬'을 만듭니다.
			FQuat Rotation180 = FQuat(FRotator(0.f, 180.f, 0.f));

			// 3. 메쉬의 월드 트랜스폼을 액터 기준의 로컬 공간으로 옮깁니다.
			// 이 작업이 "액터 중심을 기준으로 삼겠다"는 선언과 같습니다.
			FTransform MeshRelativeInActor = MeshW.GetRelativeTransform(ActorW);

			// 4. 로컬 공간에서 회전을 적용합니다.
			// 메쉬의 위치(Location)와 회전(Rotation) 모두 액터 중심을 기준으로 180도 돌아갑니다.
			FTransform RotatedRelative;
			RotatedRelative.SetLocation(Rotation180.RotateVector(MeshRelativeInActor.GetLocation()));
			RotatedRelative.SetRotation(Rotation180 * MeshRelativeInActor.GetRotation());

			// 5. 다시 월드 좌표로 변환하여 최종 타겟을 구합니다.
			FTransform FinalTargetW = RotatedRelative * ActorW;

			UE_LOG(LogTemp, Log, TEXT("Set CCMClosed Transform Dot-"));
			CCMClosed->SetActorTransform(FinalTargetW);
		}

		CCMClosed->PlayCached();
	}
	else if(DoorOpenType == EDoorOpenType::Opened_A)
	{
		UE_LOG(LogTemp, Log, TEXT("Set CCMOpen Transform to Opened A"));

		FTransform ActorW = GetActorTransform(); 
		FTransform MeshW = Mesh0->GetComponentTransform();

		FQuat Rotation180 = FQuat(FRotator(0.f, 180.f, 0.f));

		FTransform MeshRelativeInActor = MeshW.GetRelativeTransform(ActorW);

		FTransform RotatedRelative;
		RotatedRelative.SetLocation(Rotation180.RotateVector(MeshRelativeInActor.GetLocation()));
		RotatedRelative.SetRotation(Rotation180 * MeshRelativeInActor.GetRotation());

		FTransform FinalTargetW = RotatedRelative * ActorW;

		CCMOpened->SetActorTransform(FinalTargetW);

		CCMOpened->PlayCached();
	}
	else // DoorOpenType == EDoorOpenType::Opened_B
	{
		UE_LOG(LogTemp, Log, TEXT("Set CCMOpen Transform to Opened B"));

		CCMOpened->PlayCached();
	}
}

/*
*
*/
void APGDoor1::HighlightOn() const
{
	Mesh0->SetRenderCustomDepth(true);
}

/*
* 
*/
void APGDoor1::HighlightOff() const
{
	Mesh0->SetRenderCustomDepth(false);
}

/*
* 일반 Door의 InteractionInfo 반환
* HoldInput
* 잠김 -> 1초 홀드
* 잠김 x -> 0.3초 홀드
*/
FInteractionInfo APGDoor1::GetInteractionInfo() const
{
	const float Duration = bIsLocked ? 1.0f : 0.3f;
	return FInteractionInfo(EInteractionType::Hold, Duration);
}

/*
* 잠긴 문인 경우
*	상호작용 시도 플레이어가 Key를 들고 있으면 상호작용 가능
*	상호작용 시도 플레이어가 Key를 들고 있지 않으면 상호작용 불가능, 실패 메시지 return
* 잠기지 않은 문인 경우
*	상호작용 가능
*/
bool APGDoor1::CanStartInteraction(UAbilitySystemComponent* InteractingASC, FText& OutFailureMessage) const
{
	if (bIsLocked)
	{
		if (InteractingASC && InteractingASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Item.Consumable.Key"))))
		{
			return true;
		}
		OutFailureMessage = FText::FromString(TEXT("Door is locked"));

		return false;
	}

	return true;
}

void APGDoor1::InteractionFailed()
{
	PlayDoorSound(LockedDoorSound);

	Multicast_ActivateShakeEffect();
}

void APGDoor1::OnRep_DesiredTransform()
{
	DoorHinge->SetRelativeTransform(DesiredTransform);
}

// Client action after change lock state
void APGDoor1::OnRep_LockState()
{
	UE_LOG(LogTemp, Log, TEXT("Door lock state changed: %s"), bIsLocked ? TEXT("Locked") : TEXT("Unlocked"));
}

void APGDoor1::UnLock()
{
	PlayDoorSound(DoorUnlockSound);

	bIsLocked = false; 
	OnRep_LockState();
}

void APGDoor1::PlayDoorSound(const FName& SoundName, const bool IsEnemyHear)
{
	if (ISoundManagerInterface* GameModeSoundManagerInterface = Cast<ISoundManagerInterface>(GetWorld()->GetAuthGameMode()))
	{
		if (APGSoundManager* SoundManager = GameModeSoundManagerInterface->GetSoundManager())
		{
			if (IsEnemyHear)
			{
				SoundManager->PlaySoundWithNoise(SoundName, GetActorLocation());
			}
			else
			{
				SoundManager->PlaySoundForAllPlayers(SoundName, GetActorLocation());
			}
		}
	}
}

void APGDoor1::TEST_OpenDoorByAI(AActor* InteractInvestigator)
{
	UE_LOG(LogTemp, Log, TEXT("OpenDoor by AI"));
	SetDoorState(true, InteractInvestigator);
}