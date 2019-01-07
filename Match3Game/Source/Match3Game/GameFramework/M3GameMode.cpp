// Fill out your copyright notice in the Description page of Project Settings.

#include "M3GameMode.h"

AM3GameMode::AM3GameMode(const FObjectInitializer& ObjectInitializer)
{
}

void AM3GameMode::BeginPlay()
{
}

void AM3GameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
}

void AM3GameMode::GameRestart()
{
}

void AM3GameMode::GameOver()
{
}

bool AM3GameMode::IsGameActive() const
{
	return false;
}

void AM3GameMode::PauseGameTimer(bool bPause)
{
}

FString AM3GameMode::GetRamainingTimeAsString()
{
	return FString();
}

bool AM3GameMode::GetTimerPaused()
{
	return false;
}

void AM3GameMode::AddScore(int32 Points)
{
}

void AM3GameMode::SetComboPower(int32 NewComboPower)
{
}

int32 AM3GameMode::GetComboPower()
{
	return int32();
}

int32 AM3GameMode::GetMaxComboPower()
{
	return int32();
}

int32 AM3GameMode::CalculateBombPower_Implementation()
{
	return int32();
}

void AM3GameMode::UpdateScoreFromLeaderBoard(int32 GoldScore, int32 SilverScore, int32 BronzeScore)
{
}
