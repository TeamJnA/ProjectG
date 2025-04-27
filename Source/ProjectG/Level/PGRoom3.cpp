// Fill out your copyright notice in the Description page of Project Settings.


#include "PGRoom3.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"

APGRoom3::APGRoom3()
{
	static ConstructorHelpers::FClassFinder<AActor> MeshRef(TEXT("/Script/Engine.Blueprint'/Game/BP/LI_LabSample.LI_LabSample_C'"));

	RoomDir->SetRelativeLocation(FVector(900.0f, 0.0f, 130.0f));

	OverlapBox->SetRelativeLocation(FVector(1670.0f, 0.0f, 220.0f));
	OverlapBox->SetRelativeScale3D(FVector(51.0f, 21.0f, 6.5f));
	
	Mesh = CreateDefaultSubobject<UChildActorComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GeometryFolder);
	Mesh->SetChildActorClass(MeshRef.Class);
	Mesh->SetRelativeLocation(FVector(1674.0f, 0.0f, -8.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
}
