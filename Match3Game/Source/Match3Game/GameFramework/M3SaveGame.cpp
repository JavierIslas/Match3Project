// Fill out your copyright notice in the Description page of Project Settings.

#include "M3SaveGame.h"
#include "Kismet/GameplayStatics.h"

bool UM3SaveGame::LoadCustomInt(FString FieldName, int32 & Value) const
{
	const int32* ValuePointer = Match3CustomIntData.Find(FieldName);
	if (ValuePointer != nullptr)
	{
		Value = *ValuePointer;
		return true;
	}
	return false;
}

void UM3SaveGame::SaveCustomInt(FString FieldName, int32 Value)
{
	Match3CustomIntData.FindOrAdd(FieldName) = Value;
}

void UM3SaveGame::ClearCustomInt(FString FieldName)
{
	Match3CustomIntData.Remove(FieldName);
}
