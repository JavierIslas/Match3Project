// Fill out your copyright notice in the Description page of Project Settings.

#include "M3BlueprintFunctionLibrary.h"
#include "M3PlayerController.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "M3GameMode.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"


APlayerController* UM3BlueprintFunctionLibrary::GetLocalPlayerController(UObject* WorldContexObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContexObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PC = Iterator->Get();
			if (PC && PC->IsLocalController())
			{
				//For now there's onle one local player
				return PC;
			}
		}
	}
	return nullptr;
}

FString UM3BlueprintFunctionLibrary::GetOnlineAccoountID(AM3PlayerController * PlayerController)
{
	if (PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->UniqueId.IsValid())
	{
		return PlayerController->PlayerState->UniqueId->GetHexEncodedString();
	}
	return FString();
}

bool UM3BlueprintFunctionLibrary::IsGameActive(UObject * WorldContexObject)
{
	if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(WorldContexObject)))
	{
		if (GM->IsGameActive())
		{
			return true;
		}
	}
	return false;
}

void UM3BlueprintFunctionLibrary::PauseGameTimer(UObject * WorldContexObject, bool bPause)
{
	if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(WorldContexObject)))
	{
		GM->PauseGameTimer(bPause);
	}
}
