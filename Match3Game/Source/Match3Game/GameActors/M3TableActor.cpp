// Fill out your copyright notice in the Description page of Project Settings.
/**TODO CHANGE CHECKS for IF's*/
#include "M3TableActor.h"
#include "M3TileActor.h"
#include "PaperSpriteComponent.h"
#include "Math/UnrealMath.h"
#include "../GameFramework/M3GameMode.h"
#include "../GameFramework/M3BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AM3TableActor::AM3TableActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MinimumRunLength = 3;
	TileSize.Set(25.0f, 25.0f);

}

AM3TileActor * AM3TableActor::CreateTile(TSubclassOf<AM3TileActor> TileToSpawn, UMaterialInterface * TileMaterial, FVector SpawnLocation, int32 SpawnTableAddress, int32 TileTypeID)
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

void AM3TableActor::OnSwapDisplayFinished(AM3TileActor * InTile)
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
	for (int32 x = 0; x < TableWidth; ++x)
	{
		//Replace all null tiles, starting from the top of the column
		//Stop when hit a non-null tile
		int32 BaseAddress, TestAddress;
		if (GetTableAddressWithOffset(0, x, TableHeight - 1, BaseAddress))
		{
			int32 YDepth;
			for (YDepth = 0; GetTableAddressWithOffset(BaseAddress, 0, -YDepth, TestAddress) && (!GetTileFromTableAddress(TestAddress)); ++YDepth)
			{
				//This loop finds the lowest Y value, but does nothing TODO SOMETHING
			}
			for (int32 Y = YDepth; Y >= 0 ; --Y)
			{
				int32 NewTileTypeID = SelectTileFromLibrary();
				GetTableAddressWithOffset(BaseAddress, 0, -Y, TestAddress);
				//Move our rile up visually so it has room to fall, but don't change its grid address. The new grid address would be off-table and invalid anyway
				if (AM3TileActor* NewTile = CreateTile(TileLibrary[NewTileTypeID].TileClass, TileLibrary[NewTileTypeID].TileMaterial, GetLocationFromTableAddressWithOffset(TestAddress, 0, (YDepth + 1)), TestAddress, NewTileTypeID))
				{
					TilesToCheck.Add(NewTile);
					NewTile->TileState = ETileState::ETS_Falling;
					check(!FallingTiles.Contains(NewTile));
					FallingTiles.Add(NewTile);
				}
			}
		}
		else
		{
			check(false);
		}
	}
	if (FallingTiles.Num() > 0)
	{
		//Any falling tile that exist at this point are new ones, and are falling from physical location (off table) to their correct location
		for (AM3TileActor* Tile : FallingTiles)
		{
			Tile->StartFalling(true);
		}
		return;
	}

	//Check for automatic matches 
	TArray<AM3TileActor*> AllMatchingTiles;
	for (AM3TileActor* Tile : TilesToCheck)
	{
		TArray<AM3TileActor*> MatchingTiles = FindNeighbors(Tile);
		for (AM3TileActor* MatchingTile : MatchingTiles)
		{
			AllMatchingTiles.AddUnique(MatchingTile);
		}
	}

	if (AllMatchingTiles.Num() > 0)
	{
		SetLastMove(EMatch3MoveType::MT_Combo);
		ExecuteMatch(AllMatchingTiles);
	}
	else
	{
		if (IsUnWinnable())
		{
			if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(this)))
			{
				GM->GameOver();
				return;
			}
		}
		//UM3BlueprintFunctionLibrary::PauseGameTimer(this, false);
	}
}

void AM3TableActor::SwapTiles(AM3TileActor * A, AM3TileActor * B, bool bRepositionTileActors)
{
	//Swap Table positions for A and B
	int32 TableAddress = A->GetTableAddress();
	A->SetTableAddress(B->GetTableAddress());
	B->SetTableAddress(A->GetTableAddress());

	//Spawn array positions for A and B
	GameTiles[A->GetTableAddress()] = A;
	GameTiles[B->GetTableAddress()] = B;

	if (bRepositionTileActors)
	{
		//Move tiles to their new positions
		A->SetActorLocation(GetLocationFromTableAddress(A->GetTableAddress()));
		B->SetActorLocation(GetLocationFromTableAddress(B->GetTableAddress()));
	}

}

bool AM3TableActor::IsMoveLegal(AM3TileActor * A, AM3TileActor * B)
{
	if (A && B && (A->TileTypeID != B->TileTypeID) && AreAddressesNeighbors(A->GetTableAddress(), B->GetTableAddress()))
	{
		if ((A->TileTypeID != B->TileTypeID) && AreAddressesNeighbors(A->GetTableAddress(), B->GetTableAddress()))
		{
			SwapTiles(A, B);

			//Check for matches with A and B in their proposed positions
			LastLegalMatch = FindNeighbors(A);
			LastLegalMatch.Append(FindNeighbors(B));

			SwapTiles(A, B);

			return (LastLegalMatch.Num() > 0);
		}
	}
	return false;
}

TArray<AM3TileActor*> AM3TableActor::GetExplosionList(AM3TileActor * A) const
{
	check(A);
	check(A->Abilities.CanExplode());
	int32 AdjustedBombPower = A->Abilities.BombPower;
	if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(A)))
	{
		AdjustedBombPower = FMath::Max(1, AdjustedBombPower + 1 + GM->CalculateBombPower());
	}
	return FindNeighbors(A, false, AdjustedBombPower);
}

TArray<AM3TileActor*> AM3TableActor::FindNeighbors(AM3TileActor * StartingTile, bool bMustMatchID, int32 RunLength) const
{
	int32 NeighborTableAddress;
	AM3TileActor* NeighborTile;
	TArray<AM3TileActor*> MatchInProgress;
	TArray<AM3TileActor*> AllMatchingTiles;

	if (RunLength < 0)
	{
		RunLength = MinimumRunLength;
	}

	//Handle special, trival cases
	if (RunLength == 0)
	{
		return AllMatchingTiles;
	}
	else if (RunLength == 1)
	{
		AllMatchingTiles.Add(StartingTile);
		return AllMatchingTiles;
	}

	//Check vertical, then horizontal
	for (int32 Horizontal = 0; Horizontal < 2; ++Horizontal)
	{
		//Check negative possitions, then positives
		for (int32 Direction = -1; Direction <= 1; Direction += 2)
		{
			int32 MaxTableOffset = !bMustMatchID ? RunLength : (Horizontal ? TableWidth : TableHeight);
			//Check run length. A run ends when we go oof the table or hit a non matching tile, provided we care about matching
			for (int32 TableOffset = -1; TableOffset < MaxTableOffset; ++TableOffset)
			{
				if (GetTableAddressWithOffset(StartingTile->GetTableAddress(), Direction * (Horizontal ? TableOffset : 0), Direction * (Horizontal ? 0 : TableOffset), NeighborTableAddress))
				{
					NeighborTile = GetTileFromTableAddress(NeighborTableAddress);
					if (NeighborTile && (!bMustMatchID || (NeighborTile->TileTypeID == StartingTile->TileTypeID)))
					{
						MatchInProgress.Add(NeighborTile);
						continue;
					}
					break;
				}
			}
		}

		//See if we have enough to complete a run or if a matching wasn't requiered. If so keep the tiles. Note that we add 1 to our MatchInProgress because the starting tile isn't counted yet
		if (!bMustMatchID || ((MatchInProgress.Num() + 1) >= FMath::Min(RunLength, Horizontal ? TableWidth : TableHeight)))
		{
			AllMatchingTiles.Append(MatchInProgress);
		}
		MatchInProgress.Empty();
	}
	//If we found any other tile, or if we're not conserner with matching TileID, then we know we have a valid run, and we need to add the original tile to the list
	//If we do care about matchin tile type and we haven't found anything by this point, then we don't have a match and should not return the starting tile in a list by itself
	if (AllMatchingTiles.Num() > 0 || !bMustMatchID)
	{
		AllMatchingTiles.Add(StartingTile);
	}
	return AllMatchingTiles;
}

TArray<AM3TileActor*> AM3TableActor::FindTilesofType(int32 TileTypeID) const
{
	TArray<AM3TileActor*> ReturnList;
	for (AM3TileActor* Tile : GameTiles)
	{
		if (Tile && (Tile->TileTypeID == TileTypeID))
		{
			ReturnList.Add(Tile);
		}
	}
	return ReturnList;
}
//const because it'll never chance the content of the array inside this function
//reference (&) because it's not needed to make a local copy, and it's often beter for the performance to avoid copy
void AM3TableActor::ExecuteMatch(const TArray<AM3TileActor*>& MatchingTiles)
{
	if (MatchingTiles.Num() == 0)
	{
		return;
	}
	//UM3BlueprintFunctionLibrary::PauseGameTimer(this, true);

	//Destroy all tiles in MatchinTiles and award points
	for (AM3TileActor* Tile : MatchingTiles)
	{
		//Tell all the above tiles that they need to fall. -Y on the table
		int32 NextAddressUp;
		AM3TileActor* NextTileUp;
		for (int32 YOffset = 1; YOffset < TableHeight; ++YOffset)
		{
			if (GetTableAddressWithOffset(Tile->GetTableAddress(), 0, YOffset, NextAddressUp))
			{
				NextTileUp = GetTileFromTableAddress(NextAddressUp);
				//If tile invalid or being destroyed, stop adding to the list
				if (NextTileUp && !MatchingTiles.Contains(NextTileUp))
				{
					//Set the tile for falling state as soon it's added to the list
					NextTileUp->TileState = ETileState::ETS_Falling;
					check(!FallingTiles.Contains(NextTileUp));
					FallingTiles.Add(NextTileUp);
					continue;
				}
				break;
			}
		}
		Tile->TileState = ETileState::ETS_PendingDelete;
	}

	//Establish number of tiles it's needed to check after moving tiles and refilling the table
	TilesToCheck.Reset(FallingTiles.Num() + MatchingTiles.Num());

	//Add score based on tile count. TODO Borrar
	if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(this)))
	{
		EMatch3MoveType::Type MT = GetLastMove();
		int32 ScoreMult = GetScoreMultiplierForMove(MT);
		//Special results for certain move types
		switch (MT)
		{
		case EMatch3MoveType::MT_Combo:
			//PowerUp Combo
			GM->SetComboPower(FMath::Min(GM->GetMaxComboPower(), GM->GetComboPower() + 1));
			break;
		case EMatch3MoveType::MT_Bomb:
		case EMatch3MoveType::MT_AllBombs:
			//Clear Combo when bombing
			GM->SetComboPower(0);
			break;
		default:
			break;
		}
		OnMoveMade(MT);
		GM->AddScore(MatchingTiles.Num() * ScoreMult);
	}

	for (AM3TileActor* Tile : MatchingTiles)
	{
		TilesBeingDestroyed.Add(Tile);
		GameTiles[Tile->GetTableAddress()] = nullptr;
		Tile->OnMatched(GetLastMove());
	}
	//

	//Check in case there are no tiles to destroy
	OnTileFinishedMatching(nullptr);
}

void AM3TableActor::OnTileWasSelected(AM3TileActor * NewSelectedTile)
{
	//Can't select tiles while tiles are animating/moving, or game is not active
	if (FallingTiles.Num() || TilesBeingDestroyed.Num() || bPendingSwapMove || !NewSelectedTile) // || !UM3BlueprintFunctionLibrary::IsGameActive(this)
	{
		return;
	}

	FTileType& NewSelectedTileType = TileLibrary[NewSelectedTile->TileTypeID];
	if (CurrentlySelectedTile)
	{
		//Selecting a neighbor results in attemting a move
		if (AreAddressesNeighbors(CurrentlySelectedTile->GetTableAddress(), NewSelectedTile->GetTableAddress()))
		{
			if (NewSelectedTileType.Abilities.CanSwap())
			{
				bPendingSwapMove = true;
				bPendingSwapMoveSuccess = (IsMoveLegal(CurrentlySelectedTile, NewSelectedTile));
				CurrentlySelectedTile->OnSwapMove(NewSelectedTile, bPendingSwapMoveSuccess);
				NewSelectedTile->OnSwapMove(CurrentlySelectedTile, bPendingSwapMoveSuccess);
			}
			else
			{
				//Indicate failure because the second tile was not movable. Deselect the tile we were trying to swap with it
				OnMoveMade(EMatch3MoveType::MT_Failure);
			}
		}

		//Whatever happened whit the selected tiles, they are no longer selected
		CurrentlySelectedTile->PlaySelectionEffect(false);
		CurrentlySelectedTile = nullptr;
	}
	else
	{
		//Check for various special abilities on the (single) selected tile
		if (NewSelectedTileType.Abilities.CanExplode())
		{
			TArray<AM3TileActor*> TilesToDestroy;
			if (AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(this)))
			{
				if (GM->GetComboPower() == GM->GetMaxComboPower())
				{
					//Detonate all bombs at once
					SetLastMove(EMatch3MoveType::MT_AllBombs);
					// If we had multiple bomb types, this would only find the type of bomb we clcked on, because we're matching by checking TileTypeID insted of bCanExplode
					TArray<AM3TileActor*> Bombs = FindTilesofType(NewSelectedTile->TileTypeID);
					TArray<AM3TileActor*> TilesToDestroyForCurrentBomb;
					for (AM3TileActor* Bomb : Bombs)
					{
						TilesToDestroyForCurrentBomb = GetExplosionList(Bomb);
						for (AM3TileActor* TilesToCheck : TilesToDestroyForCurrentBomb)
						{
							//Don't add tiles that are already covered by a diferent bomb
							TilesToDestroy.Add(TilesToCheck);
						}
					}
				}
			}
			if (TilesToDestroy.Num() == 0)
			{
				//Regular bomb detonation, need to establish a list of tiles to destroy
				SetLastMove(EMatch3MoveType::MT_Bomb);
				TilesToDestroy = GetExplosionList(NewSelectedTile);
			}
			ExecuteMatch(TilesToDestroy);
		}
		else if (NewSelectedTileType.Abilities.CanSwap())
		{
			//This is the first tile in the sequence, so remember it for later
			CurrentlySelectedTile = NewSelectedTile;
			CurrentlySelectedTile->PlaySelectionEffect(true);
		}
		else
		{
			//Indicate failure because the first tile has no usable abilities
			OnMoveMade(EMatch3MoveType::MT_Failure);
		}
	}
}

bool AM3TableActor::IsUnWinnable()
{
	for (AM3TileActor* Tile : GameTiles)
	{
		check(Tile);
		int32 TileTableAddress = Tile->GetTableAddress();
		int32 NeighborTableAddress;
		//Bombs are always valid
		if (Tile->Abilities.CanExplode()) { return false; }
		//If any tile can move in any direction, then the game is not unwinnable
		if (GetTableAddressWithOffset(TileTableAddress, 0, -1, NeighborTableAddress)
			&& IsMoveLegal(Tile, GetTileFromTableAddress(NeighborTableAddress))) { return false; }
		if (GetTableAddressWithOffset(TileTableAddress, 0, 1, NeighborTableAddress)
			&& IsMoveLegal(Tile, GetTileFromTableAddress(NeighborTableAddress))) { return false; }
		if (GetTableAddressWithOffset(TileTableAddress, -1, 0, NeighborTableAddress)
			&& IsMoveLegal(Tile, GetTileFromTableAddress(NeighborTableAddress))) { return false; }
		if (GetTableAddressWithOffset(TileTableAddress, 1, 0, NeighborTableAddress)
			&& IsMoveLegal(Tile, GetTileFromTableAddress(NeighborTableAddress))) { return false; }
	}
	//No powerips or other non-tile moves are available, and no tiles can move in any direction
	return true;
}

void AM3TableActor::SetLastMove(EMatch3MoveType::Type MoveType)
{
	if (APlayerController* PC = UM3BlueprintFunctionLibrary::GetLocalPlayerController(this))
	{
		//Find (or add) the entry for this PlayerController and set it to the type of move that was just made
		//For multiplayer purposes but it's work on single player
		EMatch3MoveType::Type& LastMoveType = LastMove.FindOrAdd(PC);
		LastMoveType = MoveType;
	}
}

EMatch3MoveType::Type AM3TableActor::GetLastMove()
{
	//Retrieve the type of move most recently made by the given player
	//This could be stored as a single variable instead of a TMapif we were certain that our game would never support multiplayer
	if (APlayerController* PC = UM3BlueprintFunctionLibrary::GetLocalPlayerController(this))
	{
		if (EMatch3MoveType::Type* MoveType = LastMove.Find(PC))
		{
			return *MoveType;
		}
	}
	return EMatch3MoveType::MT_None;
}

int32 AM3TableActor::GetScoreMultiplierForMove_Implementation(EMatch3MoveType::Type LastMoveType)
{
	//Default value per action
	return 100;
}