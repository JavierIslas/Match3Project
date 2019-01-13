// Fill out your copyright notice in the Description page of Project Settings.

#include "M3PlayerController.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AM3PlayerController::AM3PlayerController(const FObjectInitializer& ObjectInitializer)
{
	bShowMouseCursor = true;
	
	bEnableTouchEvents = true;
	bEnableTouchOverEvents = true;
	
	ScoreChangeRate = 375.0f;
}

void AM3PlayerController::AddScore(int32 Points, bool bForceImmediateUpdate)
{
	Score += Points;
	if (bForceImmediateUpdate)
	{
		DisplayScore = Score;
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerHandle_TickScoreDisplay, this, &AM3PlayerController::TickScoreDisplay, 0.001f, true);
	}
}

int32 AM3PlayerController::GetScore()
{
	return Score;
}

int32 AM3PlayerController::GetDisplayedScore()
{
	return DisplayScore;
}

int32 AM3PlayerController::CalculateBombPower_Implementation()
{
	return 0;
}

void AM3PlayerController::TickScoreDisplay()
{
	//Assumes score only goes up, or instantly drops when it is decresed
	DisplayScore += UGameplayStatics::GetWorldDeltaSeconds(this) * ScoreChangeRate;
	if (DisplayScore >= Score)
	{
		DisplayScore = Score;
		GetWorldTimerManager().ClearTimer(TimerHandle_TickScoreDisplay);
	}
}
