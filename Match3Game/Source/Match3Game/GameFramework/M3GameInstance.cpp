// Fill out your copyright notice in the Description page of Project Settings.

#include "M3GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "CoreDelegates.h"

UM3GameInstance::UM3GameInstance()
{
	DefaultSaveGameSlot = TEXT("_Match3Game");
}

void UM3GameInstance::Init()
{
	//Point to a default save slot at startup. We'll later change our save slot when we log in
	InitSaveGameSlot();

	LoginChangedHandle = FCoreDelegates::OnUserLoginChangedEvent.AddUObject(this, &UM3GameInstance::OnLoginChanged);
	EnteringForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UM3GameInstance::OnEnteringForeground);
	EnteringBackgroundHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UM3GameInstance::OnEnteringBackground);
	ViewportHandle = FViewport::ViewportResizedEvent.AddUObject(this, &UM3GameInstance::OnViewportResize_Internal);

	Super::Init();
}

void UM3GameInstance::Shutdown()
{
	FCoreDelegates::OnUserLoginChangedEvent.Remove(LoginChangedHandle);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(EnteringForegroundHandle);
	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(EnteringBackgroundHandle);
	FViewport::ViewportResizedEvent.Remove(ViewportHandle);

	Super::Shutdown();
}

void UM3GameInstance::InitSaveGameSlot()
{
	const FString SaveSlotName = GetSaveSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		//Clear default save file, if it exists
		if (UGameplayStatics::DoesSaveGameExist(DefaultSaveGameSlot, 0))
		{
			UGameplayStatics::DeleteGameInSlot(DefaultSaveGameSlot, 0);
		}
		//If no save object, create one
		if (!InstanceGameData)
		{
			//Either not logged in with an Online ID, or we have no save data to transfer over (usually, this indecates program startup)
			InstanceGameData = Cast<UM3SaveGame>(UGameplayStatics::CreateSaveGameObject(UM3SaveGame::StaticClass()));
		}
		UGameplayStatics::SaveGameToSlot(InstanceGameData, SaveSlotName, 0);
	}
	else
	{
		InstanceGameData = Cast<UM3SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	}
	check(InstanceGameData);
}

bool UM3GameInstance::FindSaveDataForLevel(UObject * WorldContextObject, FMatch3LevelSaveData & OutSaveData)
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(WorldContextObject, true);
	if (FMatch3LevelSaveData* FoundData = InstanceGameData->Match3SaveData.Find(LevelName))
	{
		OutSaveData = *FoundData;
		return true;
	}
	return false;
}

void UM3GameInstance::SaveGame()
{
	UGameplayStatics::SaveGameToSlot(InstanceGameData, GetSaveSlotName(), 0);
}

bool UM3GameInstance::LoadCustomInt(FString FileName, int32 & Value)
{
	check(InstanceGameData);
	return InstanceGameData->LoadCustomInt(FileName, Value);
}

void UM3GameInstance::SaveCustomInt(FString FieldName, int32 Value)
{
	check(InstanceGameData);
	InstanceGameData->ClearCustomInt(FieldName);
}

void UM3GameInstance::ClearCustomInt(FString FieldName)
{
	check(InstanceGameData);
	InstanceGameData->ClearCustomInt(FieldName);
}

void UM3GameInstance::UpdateSave(UObject * WorldCOntextObject, FMatch3LevelSaveData & NewData)
{
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(WorldCOntextObject, true);
	InstanceGameData->Match3SaveData.FindOrAdd(LevelName) = NewData;
	UpdateUIAfterSave();
}

void UM3GameInstance::RegisterOnlineID(FString NewOnlineID)
{
	SaveGamePrefix = NewOnlineID;
	InitSaveGameSlot();
}

void UM3GameInstance::OnViewportResize_Internal(FViewport * Viewport, uint32 ID)
{
	OnViewportResize();
}

FString UM3GameInstance::GetSaveSlotName() const
{
	return SaveGamePrefix + DefaultSaveGameSlot;
}
