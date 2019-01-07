// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "M3SaveGame.generated.h"

USTRUCT(BlueprintType)
struct FMatch3LevelSaveData
{
	GENERATED_USTRUCT_BODY()

	/** Score to beat to get gold */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 GoldScore;

	/** Score to beat to get silver */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Silvercore;

	/** Score to beat to get bronze */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BronzeScore;

	/** Player's personal best score. Not necessarily a gold-medal score */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TopScore;
};


UCLASS()
class MATCH3GAME_API UM3SaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TMap<FString, FMatch3LevelSaveData> Match3SaveData;

	/** Load the int32 value associated with the requested variable */
	bool LoadCustomInt(FString FieldName, int32& Value) const;
	
	/** Create a variable in the save game and associate the provided integer value with it */
	void SaveCustomInt(FString FieldName, int32 Value);

	/** Erase a variable from the save game */
	void ClearCustomInt(FString FieldName);

protected:
	UPROPERTY()
	TMap<FString, int32> Match3CustomIntData;
};
