// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PGSaveGame.h"
#include "Misc/SecureHash.h"


namespace
{
	const FString PGSaveSecret = TEXT("PGSaveSecret_9f3a");
}

FString UPGSaveGame::ComputeSignature() const
{
	const FString Payload = FString::Printf(TEXT("%lld|%d|%d|%s"), TotalXP, RankIndex, GamesCompleted, *PGSaveSecret);
	return FMD5::HashAnsiString(*Payload);
}

bool UPGSaveGame::VerifySignature() const
{
	return Signature == ComputeSignature();
}
