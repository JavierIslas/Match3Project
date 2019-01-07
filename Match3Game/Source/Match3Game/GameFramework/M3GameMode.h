// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Blueprint/UserWidget.h"
#include "../GameActors/M3TableActor.h"
#include "../GameActors/M3TileActor.h"
#include "M3SaveGame.h"
#include "M3GameMode.generated.h"

USTRUCT(BlueprintType)
struct FMatch3Reward
{
	GENERATED_USTRUCT_BODY()

	/** Reward triggers at this interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreInterval;

	/** Reward grants this much time upon triggering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeAwarded;
};

UCLASS()
class MATCH3GAME_API AM3GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AM3GameMode(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/** Remove the current menu widget and vreate a new one from the specified class, if provided */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ChangeMenuWidget(TSubclassOf<UUserWidget> NewWidgetClass);

	/** How quickly tiles slide into place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	float TileMoveSpeed;

	/** Function to call when starting a new Game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void GameRestart();

	/** Function to call when the game ends */
	void GameOver();

	/** Function to indentify whether or not game is currently being player */
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool IsGameActive() const;

	/** Function to pause timer (when things are moving) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void PauseGameTimer(bool bPause);

	/** Rewards that happen at intervals during the game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	TArray<FMatch3Reward> Rewards;

	/**Get remaining game time .TODO Borrar */
	UFUNCTION(BlueprintCallable, Category = "Game")
	FString GetRamainingTimeAsString();

	UFUNCTION(BlueprintCallable, Category = "Game")
	bool GetTimerPaused();

	/** Notifies when the player has taken a new place. e.g. 1st place, by beating one of the current high score. If called with 0, just a regular scoring event. -1 = lose */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void AwardPlace(int32 NewPlace, int32 PointsGiven);

	/** Notifies when the player has received a bonus/reward, currently a time increase */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void AwardBonus();

	/** The game mode handles point-scoring */
	void AddScore(int32 Points);

	/** The game mode understand the concept of combo power */
	void SetComboPower(int32 NewComboPower);

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetComboPower();

	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetMaxComboPower();

	/** Bomb power requst (current) - susceptible to BP overriding by the PlayerController */
	UFUNCTION(BlueprintNativeEvent, Category = "Game")
	int32 CalculateBombPower();
	virtual int32 CalculateBombPower_Implementation();

	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	void GameWasWon(bool bGameWasWon);

	/** Used to force update the score in the save data, say from the leader boards */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void UpdateScoreFromLeaderBoard(int32 GoldScore, int32 SilverScore, int32 BronzeScore);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FMatch3LevelSaveData SaveData;

	UPROPERTY(BlueprintReadOnly, Category = "Score")
	int32 FinalPlace;

protected:
	/** The Widget class we will use as our menu when the game starts */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
	TSubclassOf<UUserWidget> StartingWidgetClass;

	/** The Widget Class we will use as our game over screen when the player lose */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
	TSubclassOf<UUserWidget> DefeatWidgetClass;

	/** Use as Game Over screen when the player win */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	/** Widget instance that we are using as our menu */
	UPROPERTY()
	UUserWidget* CurrentWidget;

	UPROPERTY(EditDefaultsOnly)
	float TimeRemaining;

	FTimerHandle TimeHandle_GameOverTimer;

	bool GameWillBeWon;

};

