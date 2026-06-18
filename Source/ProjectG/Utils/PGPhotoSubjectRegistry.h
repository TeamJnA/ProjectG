// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PGPhotoSubjectRegistry.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGPhotoSubjectRegistry : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterSubject(AActor* Subject);
	void UnregisterSubject(AActor* Subject);
	FORCEINLINE const TArray<TWeakObjectPtr<AActor>>& GetSubjects() const { return Subjects; }

private:
	TArray<TWeakObjectPtr<AActor>> Subjects;
};
