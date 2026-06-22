// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PlayerEntry/PGPlayerProfileWidgetBase.h"
#include "PGMainMenuProfileWidget.generated.h"

class APGPlayerState;

/**
 * 
 */
UCLASS()
class PROJECTG_API UPGMainMenuProfileWidget : public UPGPlayerProfileWidgetBase
{
	GENERATED_BODY()
	
public:
	void SetupStatic(APGPlayerState* LocalPlayerState);
};
