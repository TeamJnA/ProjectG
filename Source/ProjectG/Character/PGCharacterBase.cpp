// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PGCharacterBase.h"
#include "AbilitySystem/PGAbilitySystemComponent.h"

//Sound Manager
#include "Component/PGSoundManagerComponent.h"
#include "Sound/PGSoundManager.h"

// Sets default values
APGCharacterBase::APGCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

}

UAbilitySystemComponent* APGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UPGAttributeSet* APGCharacterBase::GetAttributeSet() const
{
	return nullptr;
}

void APGCharacterBase::GiveDefaultAbilities()
{
	check(AbilitySystemComponent);
	if (!HasAuthority()) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		const FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void APGCharacterBase::InitDefaultAttributes() const
{
	if (!AbilitySystemComponent || !DefaultAttributeEffect) { return; }

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void APGCharacterBase::GiveAndActivatePassiveEffects()
{
	check(AbilitySystemComponent);
	if (!HasAuthority()) return;

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (TSubclassOf<UGameplayEffect> GameplayEffect : PassiveEffects)
	{
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1.f, EffectContext);

		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void APGCharacterBase::InitSoundManager(APGSoundManager* SoundManagerRef)
{
	SoundManagerComponent->SetSoundManager(SoundManagerRef);
	UE_LOG(LogTemp, Log, TEXT("InitSoundManager"));
}