// Fill out your copyright notice in the Description page of Project Settings.

#include "PGAbilitySystemComponent.h"

UPGAbilitySystemComponent::UPGAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}
