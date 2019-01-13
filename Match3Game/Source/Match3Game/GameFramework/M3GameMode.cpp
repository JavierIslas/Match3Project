// Fill out your copyright notice in the Description page of Project Settings.

#include "M3GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "M3PlayerController.h"
#include "M3SaveGame.h"
#include "M3GameInstance.h"
#include "TimerManager.h"
#include "M3BlueprintFunctionLibrary.h"

AM3GameMode::AM3GameMode(const FObjectInitializer& ObjectInitializer)
{
	//TODO: if performance sucks, turn this off
	PrimaryActorTick.bCanEverTick = true;
	//
	DefaultPawnClass = APawn::StaticClass();
	PlayerControllerClass = AM3PlayerController::StaticClass();
	TileMoveSpeed = 50.0f;
	TimeRemaining = 5.0f;
	FinalPlace = 0;
}

void AM3GameMode::BeginPlay()
{
	Super::BeginPlay();
	bGameWillBeWon = false;
	ChangeMenuWidget(StartingWidgetClass);
	GetWorldTimerManager().SetTimer(TimeHandle_GameOverTimer, this, &AM3GameMode::GameOver, TimeRemaining, false);

	//Get current save data from the game instance
	UM3GameInstance* GI = Cast<UM3GameInstance>(UGameplayStatics::GetGameInstance(this));
	if (GI)
	{
		//If we didn't already have save data, put our defaults into the main array. We'll save it later, if anything noteworthy happens
		if (!GI->FindSaveDataForLevel(this, SaveData))
		{
			GI->UpdateSave(this, SaveData);
		}
	}
}

void AM3GameMode::ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass)
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromViewport();
		CurrentWidget = nullptr;
	}
	if (NewWidgetClass)
	{
		if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
		{
			CurrentWidget = CreateWidget<UUserWidget>(PC, NewWidgetClass);
			if (CurrentWidget)
			{
				CurrentWidget->AddToViewport();
			}
		}
	}
}

void AM3GameMode::GameRestart()
{
	ChangeMenuWidget(nullptr);
	FName LevelName(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, LevelName);
}

void AM3GameMode::GameOver()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_GameOverTimer);
	if (bGameWillBeWon)
	{
		UM3GameInstance* GI = Cast<UM3GameInstance>(UGameplayStatics::GetGameInstance(this));
		//Check for top score
		if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
		{
			SaveData.TopScore = FMath::Max(PC->GetScore(), SaveData.TopScore);
		}
		//Save regardless of whether or not we got a high score, because we save things like number of games played
		GI->UpdateSave(this, SaveData);
		GI->SaveGame();
	}
}

bool AM3GameMode::IsGameActive() const
{
	//Game is active whenever time hasn't run out or the timer is paused
	FTimerManager& WorldTimerManager = GetWorldTimerManager();
	return (WorldTimerManager.IsTimerActive(TimeHandle_GameOverTimer) || WorldTimerManager.IsTimerPaused(TimeHandle_GameOverTimer));
}

void AM3GameMode::PauseGameTimer(bool bPause)
{
	if (bPause)
	{
		GetWorldTimerManager().PauseTimer(TimeHandle_GameOverTimer);
	}
	else
	{
		GetWorldTimerManager().UnPauseTimer(TimeHandle_GameOverTimer);
	}
}

FString AM3GameMode::GetRamainingTimeAsString()
{
	int32 OutInt = FMath::CeilToInt(GetWorldTimerManager().GetTimerRemaining(TimeHandle_GameOverTimer));
	return FString::Printf(TEXT("%03i"), FMath::Max(0, OutInt));
}

bool AM3GameMode::GetTimerPaused()
{
	return GetWorldTimerManager().IsTimerPaused(TimeHandle_GameOverTimer);
}

void AM3GameMode::AddScore(int32 Points)
{
	if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		int32 OldScore = PC->GetScore();
		PC->AddScore(Points);
		int32 NewScore = PC->GetScore();
		if (NewScore >= SaveData.BronzeScore)
		{
			bGameWillBeWon = true;
		}

		//Chech for medals
		if (NewScore > SaveData.GoldScore)
		{
			FinalPlace = 1;
			AwardPlace(1, Points);
		}
		else if (NewScore > SaveData.SilverScore)
		{
			FinalPlace = 2;
			AwardPlace(2, Points);
		}
		else if (NewScore > SaveData.BronzeScore)
		{
			FinalPlace = 3;
			AwardPlace(3, Points);
		}
		else
		{
			FinalPlace = 0;
			AwardPlace(0, Points);
		}

		for (const FMatch3Reward& Reward : Rewards)
		{
			check(Reward.ScoreInterval > 0);
			//Integer division to decide if we're crossed a bonus threshold
			int32 ScoreAwardCount = (NewScore / Reward.ScoreInterval) - (OldScore / Reward.ScoreInterval);
			if (ScoreAwardCount > 0)
			{
				float StartingTimeValue = GetWorldTimerManager().GetTimerRemaining(TimeHandle_GameOverTimer);
				if (StartingTimeValue >= 0.0f)
				{
					GetWorldTimerManager().SetTimer(TimeHandle_GameOverTimer, this, &AM3GameMode::GameOver, StartingTimeValue + (ScoreAwardCount * Reward.TimeAwarded), false);
					AwardBonus();
				}
			}
		}
	}
}

void AM3GameMode::SetComboPower(int32 NewComboPower)
{
	if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		PC->ComboPower = NewComboPower;
	}
}

int32 AM3GameMode::GetComboPower()
{
	return int32();
}

int32 AM3GameMode::GetMaxComboPower()
{
	if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		return PC->MaxComboPower;
	}
	return 0;
}

int32 AM3GameMode::CalculateBombPower_Implementation()
{
	if (AM3PlayerController* PC = Cast<AM3PlayerController>(UM3BlueprintFunctionLibrary::GetLocalPlayerController(this)))
	{
		return PC->CalculateBombPower();
	}
	return 0;
}

void AM3GameMode::UpdateScoreFromLeaderBoard(int32 GoldScore, int32 SilverScore, int32 BronzeScore)
{
	UM3GameInstance* GI = Cast<UM3GameInstance>(UGameplayStatics::GetGameInstance(this));
	SaveData.BronzeScore = BronzeScore;
	SaveData.SilverScore = SilverScore;
	SaveData.GoldScore = GoldScore;
	GI->SaveGame();
}
