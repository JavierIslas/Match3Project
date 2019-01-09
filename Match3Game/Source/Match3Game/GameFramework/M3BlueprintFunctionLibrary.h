// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "M3BlueprintFunctionLibrary.generated.h"

class AM3PlayerController;

UCLASS()
class MATCH3GAME_API UM3BlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/**Get a List of all player controllers, then pick the first local one we found */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(UObject* WorldContexObject);

	/** Get the online account ID (as an encoded hex string) associated with the provided player controller's player state. Return a blank string on failure */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay")
	static FString GetOnlineAccoountID(AM3PlayerController* PlayerController);

	/** Function to identify whether or not game is currently being played */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static bool IsGameActive(UObject* WorldContexObject);

	/** Function to identify whether or not game is currntly being played */
	UFUNCTION(BlueprintCallable, Category = "Match3 Gameplay", Meta = (WorldContext = "WorldContextObject"))
	static void PauseGameTimer(UObject* WorldContexObject, bool bPause);
};
