// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "M3PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MATCH3GAME_API AM3PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AM3PlayerController(const FObjectInitializer& ObjectInitializer);

	/** Add points. If pointsare negative or we force immediate update, the score will display instantly instead of counting up */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddScore(int32 Points, bool bForceImmediateUpdate = false);

	/** Get the actual score (not the score that is displeyed) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetScore();

	/** Get the score that is currently displayed (not the actual score) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	int32 GetDisplayedScore();

	/** Override in BP to power up bombs */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game")
	int32 CalculateBombPower();
	virtual int32 CalculateBombPower_Implementation();

	/** Current combo power */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	int32 ComboPower;

	/** Maximun combo power for this player, can be changed based on avatar. TODO: Set this from the avatar class (version 2.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Game")
	int32 MaxComboPower;

protected:
	/** Current actual score, though not the display value */
	UPROPERTY()
	int32 Score;

	/** Score that is displayed on screen (as an integer) */
	UPROPERTY()
	int32 DisplayScore;

	/** Rate at which displayed score climbs to reach actual score. Currently does not go faster with bigger scores */
	UPROPERTY(EditDefaultsOnly)
	float ScoreChangeRate;

	/** Periodic function to manage score update */
	void TickScoreDisplay();

	FTimerHandle TimerHandle_TickScoreDisplay;
};
