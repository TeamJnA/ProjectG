// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AbilityTask/AT_PGPredictThrowableTrajectory.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PGLogChannels.h"

UAT_PGPredictThrowableTrajectory::UAT_PGPredictThrowableTrajectory(const FObjectInitializer& ObjectInitializer)
{
	bTickingTask = true;
    SplineMeshesCount = 12;
}

UAT_PGPredictThrowableTrajectory* UAT_PGPredictThrowableTrajectory::DrawTrajectory(UGameplayAbility* OwningAbility, float InInitialSpeed)
{
	UAT_PGPredictThrowableTrajectory* MyObj = NewAbilityTask<UAT_PGPredictThrowableTrajectory>(OwningAbility);
    MyObj->InitialSpeed = InInitialSpeed;

	return MyObj;
}

void UAT_PGPredictThrowableTrajectory::Activate()
{
	Super::Activate();

    UE_LOG(LogPGAbilityTask, Log, TEXT("Activate UAT_PGPredictThrowableTrajectory"));

    AActor* OwningActor = GetAvatarActor();
    if (!OwningActor)
    {
        UE_LOG(LogPGAbilityTask, Warning, TEXT("Cannot find OwingActor in UAT_PGPredictThrowableTrajectory::Activate"));
        EndTask();
        return;
    }

    // 1. Spline Component 동적 생성 (OwningActor를 Outer로 지정)
    SplineComponent = NewObject<USplineComponent>(OwningActor, TEXT("SplineComponent"));

    if (!SplineComponent)
    {
        UE_LOG(LogPGAbilityTask, Warning, TEXT("Cannot create SplineComponent in UAT_PGPredictThrowableTrajectory::Activate"));
        EndTask();
        return;
    }
    // 2. OwningActor의 루트 컴포넌트에 붙임
    SplineComponent->SetupAttachment(OwningActor->GetRootComponent());

    // 3. 컴포넌트 등록
    SplineComponent->RegisterComponent();

    // Load mesh and material
    FString MeshPath = TEXT("/Game/ProjectG/Items/Common/ThrowPredict/Shape_Cylinder.Shape_Cylinder");
    CachedSplineMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *MeshPath));
    if (!CachedSplineMesh)
    {
        UE_LOG(LogPGAbilityTask, Warning, TEXT("Cannot find CachedSplineMesh in UAT_PGPredictThrowableTrajectory::Activate"));
        EndTask();
        return;
    }

    FString MaterialPath = TEXT("/Game/ProjectG/Items/Common/ThrowPredict/M_ThrowPredictMaterial.M_ThrowPredictMaterial");
    CachedSplineMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
    if (!CachedSplineMaterial)
    {
        UE_LOG(LogPGAbilityTask, Warning, TEXT("Cannot find CachedSplineMaterial in UAT_PGPredictThrowableTrajectory::Activate"));
        EndTask();
        return;
    }

    // Set spline mesh components at Active
    // It's bad to load data in tick(TickTask)
    for (int32 i = 0; i < SplineMeshesCount; i++)
    {
        USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
        SplineMeshComponents.Add(SplineMesh);
        SplineMeshComponents[i]->SetVisibility(false);
        SplineMeshComponents[i]->SetStaticMesh(CachedSplineMesh);
        SplineMeshComponents[i]->SetMaterial(0, CachedSplineMaterial);

        SplineMeshComponents[i]->SetMobility(EComponentMobility::Movable);
        SplineMeshComponents[i]->RegisterComponentWithWorld(GetWorld());
        // SplineMeshComponents[i]->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);
        SplineMeshComponents[i]->SetForwardAxis(ESplineMeshAxis::Z);

        SplineMeshComponents[i]->SetStartScale(FVector2D(0.1f, 0.1f));
        SplineMeshComponents[i]->SetEndScale(FVector2D(0.1f, 0.1f));
    }
}

void UAT_PGPredictThrowableTrajectory::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

    // Clear splines
    for (USplineMeshComponent* Mesh : SplineMeshComponents)
    {
        if (Mesh)
        {
            Mesh->SetVisibility(false);
        }
    }

    SplineComponent->ClearSplinePoints();

    // Make Predict Projectile Path
    AActor* OwningActor = GetAvatarActor();
    if (!OwningActor)
    {
        EndTask();
        return;
    }
    
    FVector EyesViewLocation;
    FRotator EyesViewRotation;
    OwningActor->GetActorEyesViewPoint(EyesViewLocation, EyesViewRotation);

    const FVector StartLocation = EyesViewLocation + (OwningActor->GetActorForwardVector() * 40) + (OwningActor->GetActorRightVector() * 25);

    EyesViewRotation.Pitch += 10;
    const FVector LaunchVector = EyesViewRotation.Vector() * InitialSpeed;

    FPredictProjectilePathParams PredictParams;

    PredictParams.StartLocation = StartLocation;
    PredictParams.LaunchVelocity = LaunchVector;
    PredictParams.bTraceWithCollision = true;
    PredictParams.ProjectileRadius = 1.0f;
    PredictParams.MaxSimTime = 0.5f;
    PredictParams.bTraceWithChannel = true;
    PredictParams.TraceChannel = ECollisionChannel::ECC_Visibility;
    PredictParams.ActorsToIgnore.Add(OwningActor);
    PredictParams.SimFrequency = 20.0f;
    PredictParams.OverrideGravityZ = 0.0f;
    PredictParams.DrawDebugType = EDrawDebugTrace::None;
    PredictParams.DrawDebugTime = 0.0f;
    PredictParams.bTraceComplex = false;

    FPredictProjectilePathResult PredictResult;

    bool bHit = UGameplayStatics::PredictProjectilePath(
        this, 
        PredictParams,
        PredictResult
    );

    for (const FPredictProjectilePathPointData& PathPointData : PredictResult.PathData)
    {
        SplineComponent->AddSplinePoint(PathPointData.Location, ESplineCoordinateSpace::World, true);
    }

    SplineComponent->SetSplinePointType(0, ESplinePointType::CurveClamped, true);

    int32 NewSplineMeshesCount = SplineComponent->GetSplineLength() / 50.0f;
    if (NewSplineMeshesCount > SplineMeshesCount)
    {
        NewSplineMeshesCount = SplineMeshesCount;
    }

    for (int32 i = 0; i < NewSplineMeshesCount; i++)
    {
        // First trajectory start with little space
        FVector PointStartLocation, PointStartTangent, PointEndLocation, PointEndTangent;
        if (i == 0)
        {
            PointStartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(15 , ESplineCoordinateSpace::World);
            PointStartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(15, ESplineCoordinateSpace::World);
        }
        else
        {
            PointStartLocation = SplineComponent->GetLocationAtDistanceAlongSpline(i * 50, ESplineCoordinateSpace::World);
            PointStartTangent = SplineComponent->GetTangentAtDistanceAlongSpline(i * 50, ESplineCoordinateSpace::World);
        }
        PointEndLocation = SplineComponent->GetLocationAtDistanceAlongSpline((i+1) * 50, ESplineCoordinateSpace::World);
        PointEndTangent = SplineComponent->GetTangentAtDistanceAlongSpline((i+1) * 50, ESplineCoordinateSpace::World);

        SplineMeshComponents[i]->SetStartAndEnd(PointStartLocation, PointStartTangent, PointEndLocation, PointEndTangent, true);
        SplineMeshComponents[i]->SetVisibility(true);
    }
}

void UAT_PGPredictThrowableTrajectory::OnDestroy(bool bInOwnerFinished)
{
    for (USplineMeshComponent* Mesh : SplineMeshComponents)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    SplineMeshComponents.Empty();
    

    if (SplineComponent)
    {
        SplineComponent->DestroyComponent();
        SplineComponent = nullptr;
    }

    Super::OnDestroy(bInOwnerFinished);
}
