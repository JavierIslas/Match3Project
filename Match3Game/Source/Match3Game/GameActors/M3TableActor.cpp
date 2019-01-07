// Fill out your copyright notice in the Description page of Project Settings.
/**TODO CHANGE CHECKS for IF's*/
#include "M3TableActor.h"
#include "M3TileActor.h"
#include "PaperSpriteComponent.h"
#include "Math/UnrealMath.h"
#include "../GameFramework/M3GameMode.h"

// Sets default values
AM3TableActor::AM3TableActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MinimumRunLength = 3;
	TileSize.Set(25.0f, 25.0f);

}

AM3TileActor * AM3TableActor::CreateTile(TSubclassOf<AM3TileActor> TileToSpawn, UMaterialInstanceConstant * TileMaterial, FVector SpawnLocation, int32 SpawnTableAddress, int32 TileTypeID)
{
	if (!TileToSpawn) { return nullptr; }
	else
	{
		checkSlow(TileLibrary.IsValidIndex(TileTypeID));

		UWorld* const World = GetWorld();
		if (!World) { return nullptr; }

		else
		{
			//Set Spawn parameters
			FActorSpawnParameters SpawnParam;
			SpawnParam.Owner = this;
			SpawnParam.Instigator = Instigator;
			SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			//Tiles never rotate
			FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

			//Spawn the tile
			AM3TileActor* const NewTile = World->SpawnActor<AM3TileActor>(TileToSpawn, SpawnLocation, SpawnRotation, SpawnParam);
			NewTile->GetRenderComponent()->SetMobility(EComponentMobility::Movable);
			NewTile->TileTypeID = TileTypeID;
			NewTile->Abilities = TileLibrary[TileTypeID].Abilities;
			NewTile->SetTileMaterial(TileMaterial);
			NewTile->SetTableAddress(SpawnTableAddress);
			GameTiles[SpawnTableAddress] = NewTile;
			return NewTile;
		}
	}
}

int32 AM3TableActor::SelectTileFromLibrary()
{
	float NormalizingFactor = 0;
	for (const FTileType& TileBase : TileLibrary)
	{
		NormalizingFactor += TileBase.Probability;
	}
	float TestNumber = FMath::FRandRange(0.0f, NormalizingFactor);
	float CompareTo = 0;
	for (int32 ArrayChecked = 0; ArrayChecked != TileLibrary.Num(); ArrayChecked++)
	{
		CompareTo += TileLibrary[ArrayChecked].Probability;
		if (TestNumber <= CompareTo)
		{
			return ArrayChecked;
		}
	}
	return 0;
}

AM3TileActor * AM3TableActor::GetTileFromTableAddress(int32 TableAddress) const
{
	if (GameTiles.IsValidIndex(TableAddress))
	{
		return GameTiles[TableAddress];
	}
	return nullptr;
}

void AM3TableActor::InitializeTable()
{
	GameTiles.Empty(TableWidth * TableHeight);
	GameTiles.AddZeroed(GameTiles.Max());
	FVector SpawnLocation;

	for (int32 Column = 0; Column < TableWidth; ++Column)
	{
		for (int32 Row = 0; Row < TableHeight; ++Row)
		{
			int32 TileID = SelectTileFromLibrary();
			int32 TableAddress;
			GetTableAddressWithOffset(0, Column, Row, TableAddress);
			SpawnLocation = GetLocationFromTableAddress(TableAddress);

			do
			{
				TileID = SelectTileFromLibrary();
				if ((Column >= MinimumRunLength - 1) || (Row >= MinimumRunLength - 1))
				{
					int32 TestAddress;
					int32 TileOffset;
					for (int32 Horizontal = 0; Horizontal < 2; ++Horizontal)
					{
						for (TileOffset = 1; TileOffset < MinimumRunLength; ++TileOffset)
						{
							if (!GetTableAddressWithOffset(0, Column - (Horizontal ? TileOffset : 0), Row - (Horizontal ? 0 : TileOffset), TestAddress) || (GetTileFromTableAddress(TestAddress)->TileTypeID != TileID))
							{
								// Not in a matching run, or off the edge of the map, so stop checking this axis
								break;
							}
						}
						if (TileOffset == MinimumRunLength)
						{
							// We made it through the whole "check for matching run" loop. This tile completes a scoring a scoring run. Pick a new tile type and test again
							break;
						}
					}
					if (TileOffset < MinimumRunLength)
					{
						// We didn't find a matching run in either direction, so we have a valid tile type
						break;
					}
				}
				else
				{
					//Tile is too close to the edge to be worth cheching
					break;
				}
			} while (true);
			CreateTile(TileLibrary[TileID].TileClass, TileLibrary[TileID].TileMaterial, SpawnLocation, TableAddress, TileID);
		}

	}
}

void AM3TableActor::ReturnMatchSounds(TArray<USoundWave*>& MatchSounds)
{
	MatchSounds.Reset();
	if (TilesBeingDestroyed.Num() > 0)
	{
		for (AM3TileActor* Tile : TilesBeingDestroyed)
		{
			MatchSounds.AddUnique(Tile->GetMatchSound());
		}
	}
}

FVector AM3TableActor::GetLocationFromTableAddress(int32 TableAddress) const
{
	FVector Center = GetActorLocation();
	FVector OutLocation = FVector(-(TableWidth * 0.5f) * TileSize.X + (TileSize.X * 0.5f), 0.0f, -(TableHeight * 0.5f) * TileSize.Y + (TileSize.Y * 0.5f));
	check(TableWidth > 0);
	OutLocation.X += TileSize.X * (float)(TableAddress % TableWidth);
	OutLocation.Z += TileSize.Y * (float)(TableAddress / TableWidth);
	OutLocation += Center;

	return OutLocation;
}

FVector AM3TableActor::GetLocationFromTableAddressWithOffset(int32 TableAddress, int32 XOffserInTiles, int32 YOffsetInTiles) const
{
	FVector OutLocation = GetLocationFromTableAddress(TableAddress);
	OutLocation.X += TileSize.X * (float)(XOffserInTiles);
	OutLocation.Y += TileSize.Y * (float)(YOffsetInTiles);
	return OutLocation;
}

bool AM3TableActor::GetTableAddressWithOffset(int32 InitialTableAddres, int32 XOffset, int32 YOffset, int32 & ReturnTableAddress) const
{
	int32 NewAxisValue;

	ReturnTableAddress = -1; //Invalid value

	//check for within X range
	check(TableWidth > 0); 
	NewAxisValue = (InitialTableAddres % TableWidth) + XOffset;
	if (NewAxisValue != FMath::Clamp(NewAxisValue, 0, (TableWidth - 1)))
	{
		return false;
	}

	//check for within Y range
	NewAxisValue = (InitialTableAddres % TableWidth) + YOffset;
	if (NewAxisValue != FMath::Clamp(NewAxisValue, 0, (TableHeight - 1)))
	{
		return false;
	}

	ReturnTableAddress = (InitialTableAddres + XOffset + (YOffset * TableWidth));
	check(ReturnTableAddress >= 0);
	check(ReturnTableAddress < (TableWidth * TableHeight));

	return true;
}

bool AM3TableActor::AreAddressesNeighbors(int32 TableAddressA, int32 TableAddressB) const
{
	if ((FMath::Min(TableAddressA, TableAddressB) >= 0) && (FMath::Max(TableAddressA, TableAddressB) < (TableWidth * TableHeight)))
	{
		int32 TableAddressOffset = FMath::Abs(TableAddressA - TableAddressB);
		return ((TableAddressOffset == 1) || (TableAddressOffset == TableWidth));
	}
	return false;
}

void AM3TableActor::OnTileFinishedFalling(AM3TileActor * Tile, int32 LandingAddress)
{
	int32 ReturnTableAddress;

	//Remove Tile from its original position if it's still there (hasn't been replaced by another falling tile)
	if (GetTableAddressWithOffset(Tile->GetTableAddress(), 0, 0, ReturnTableAddress))
	{
		if (GameTiles[ReturnTableAddress] == Tile)
		{
			GameTiles[ReturnTableAddress] = nullptr;
		}
	}

	//Validate new table address and replace whatever is there
	if (GetTableAddressWithOffset(LandingAddress, 0, 0, ReturnTableAddress))
	{
		GameTiles[ReturnTableAddress] = Tile;
		Tile->SetTableAddress(ReturnTableAddress);
		Tile->TileState = ETileState::ETS_Normal;
	}

	//Tile no longer falling
	FallingTiles.RemoveSingleSwap(Tile);
	TilesToCheck.Add(Tile);
	if (FallingTiles.Num() == 0)
	{
		RespawnTiles();
	}
}

void AM3TableActor::OnTileFinishedMatching(AM3TileActor * InTile)
{
	if (InTile)
	{
		TilesBeingDestroyed.RemoveSwap(InTile);
		InTile->Destroy();
	}

	if (TilesBeingDestroyed.Num() == 0)
	{
		//Make all the tiles fall if they are above empty space
		for (AM3TileActor* Tile : FallingTiles)
		{
			Tile->StartFalling();
		}
		if (FallingTiles.Num() == 0)
		{
			RespawnTiles();
		}
	}
}

void AM3TableActor::OnSpawnDisplayFinished(AM3TileActor * InTile)
{
	SwappingTiles.Add(InTile);
	if (SwappingTiles.Num() == 2)
	{
		check(SwappingTiles[0] && SwappingTiles[1]);
		bPendingSwapMove = false;
		if (bPendingSwapMoveSuccess)
		{
			SwapTiles(SwappingTiles[0], SwappingTiles[1], true);
			SwappingTiles.Reset();
			if (LastLegalMatch.Num() > MinimumRunLength)
			{
				SetLastMove(EMatch3MoveType::MT_MoreTiles);

			}
			else
			{
				SetLastMove(EMatch3MoveType::MT_Standard);
			}
			//Execute the (verified legal) move
			ExecuteMatch(LastLegalMatch);
		}
		else
		{
			SwappingTiles.Empty();
			OnMoveMade(EMatch3MoveType::MT_Failure);
		}
	}
}

void AM3TableActor::RespawnTiles()
{
}

void AM3TableActor::SwapTiles(AM3TileActor * A, AM3TileActor * B, bool bRepositionTileActors)
{
}

bool AM3TableActor::IsMoveLegal(AM3TileActor * A, AM3TileActor * B)
{
	return false;
}

TArray<AM3TileActor*> AM3TableActor::GetExplosionList(AM3TileActor * A) const
{
	return TArray<AM3TileActor*>();
}

TArray<AM3TileActor*> AM3TableActor::FindNeighbors(AM3TileActor * StartingTile, bool bMustMatchID, int32 RungLength) const
{
	return TArray<AM3TileActor*>();
}

TArray<AM3TileActor*> AM3TableActor::FindTilesofType(int32 TileTypeID) const
{
	return TArray<AM3TileActor*>();
}

void AM3TableActor::ExecuteMatch(const TArray<AM3TileActor*>& MatchingTiles)
{
}

void AM3TableActor::OnTileWasSelected(AM3TileActor * NewSelectedTile)
{
}

bool AM3TableActor::IsUnWinnable()
{
	return false;
}

void AM3TableActor::SetLastMove(EMatch3MoveType::Type MoveType)
{
}

EMatch3MoveType::Type AM3TableActor::GetLastMove()
{
	return EMatch3MoveType::Type();
}

int32 AM3TableActor::GetScoreMultiplierForMove_Implementation(EMatch3MoveType::Type LastMoveType)
{
	return int32();
}