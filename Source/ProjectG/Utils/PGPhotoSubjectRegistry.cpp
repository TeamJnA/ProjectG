// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/PGPhotoSubjectRegistry.h"
#include "Interface/PhotographableInterface.h"


void UPGPhotoSubjectRegistry::RegisterSubject(AActor* Subject)
{
	if (Subject && Subject->Implements<UPhotographableInterface>())
	{
		Subjects.AddUnique(Subject);
	}
}

void UPGPhotoSubjectRegistry::UnregisterSubject(AActor* Subject)
{
	Subjects.RemoveAllSwap([Subject](const TWeakObjectPtr<AActor>& Weak)
	{
		return Weak.Get() == Subject;
	});
}
