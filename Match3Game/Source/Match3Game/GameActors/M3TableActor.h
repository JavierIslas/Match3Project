// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "M3TileActor.h"
#include "M3TableActor.generated.h"

class AM3TileActor;
class UMaterialInstanceConstant;

USTRUCT(BlueprintType)
struct FTileType
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Probability = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInstanceConstant* TileMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AM3TileActor> TileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FLinearColor EddectColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FTileAbilities Abilities;
};


UCLASS()
class MATCH3GAME_API AM3TableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AM3TableActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AM3TileActor*> GameTiles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FTileType> TileLibrary;

	/** Size of a position on the Table. Doesn't have borders or spacing between tiles */
	UPROPERTY(EditDefaultsOnly, Category = "Tile")
	FVector2D TileSize;

	/** Minimun Number of matching Tiles on a row that makes a match */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 MinimumRunLength;

	/** The width of the Table. Needed to Calculate tile position and neighbors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 TableWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	int32 TableHeight;

	/** Spawn a tile and associate it with a specific Table address */
	AM3TileActor* CreateTile(TSubclassOf<AM3TileActor> TileToSpawn, UMaterialInstanceConstant* TileMaterial, FVector SpawnLocation, int32 SpawnTableAddress, int32 TileTypeID);

	/** Randomly select a type of tile from the Table's Library, using the probability values on the tile. */
	int32 SelectTileFromLibrary();

	/** Get the pointer to the tile ar the specofoed grod address. */
	AM3TileActor* GetTileFromTableAddress(int32 TableAddress) const;

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void InitializeTable();

	/** Play effcts when a move is made. USE TO AVOID SPAMMING ON TILES*/
	UFUNCTION(BlueprintImplementableEvent, meta = (ExpandEnumAsExecs = "MoveType"), Category = "Tile")
	void OnMoveMade(EMatch3MoveType::Type MoveType);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void ReturnMatchSounds(TArray<USoundWave*>& MatchSounds);

	/** Get world location from a Table address */
	UFUNCTION(BlueprintCallable, Category = "Tile")
	FVector GetLocationFromTableAddress(int32 TableAddress) const;

	/** Get world location from a Table address relative to another address. Offset between both is measured in tiles */
	FVector GetLocationFromTableAddressWithOffset(int32 TableAddress, int32 XOffserInTiles, int32 YOffsetInTiles) const;

	/** Get table address relative to another table address. Offset between both is measured in tiles */
	UFUNCTION(BlueprintCallable, Category = "Tile")
	bool GetTableAddressWithOffset(int32 InitialTableAddres, int32 XOffset, int32 YOffset, int32 &ReturnTableAddress) const;

	/** Determine if 2 table addresses are valid and adjacent */
	bool AreAddressesNeighbors(int32 TableAddressA, int32 TableAddressB) const;

	void OnTileFinishedFalling(AM3TileActor* Tile, int32 LandingAddress);

	void OnTileFinishedMatching(AM3TileActor* InTile);

	void OnSpawnDisplayFinished(AM3TileActor* InTile);

	void RespawnTiles();

	void SwapTiles(AM3TileActor* A, AM3TileActor* B, bool bRepositionTileActors = false);

	bool IsMoveLegal(AM3TileActor* A, AM3TileActor* B);

	/** Get list of tiles that will be affected by bomb's explosion TODO Borrar */
	TArray<AM3TileActor*> GetExplosionList(AM3TileActor* A) const;

	/** Check for a successful sequence. bMustMatchID can be set to ignore matching. MinimumLengthRequired will default to the game's MinimumRunLength setting if negative */
	TArray<AM3TileActor*> FindNeighbors(AM3TileActor* StartingTile, bool bMustMatchID = true, int32 RungLength = -1) const;

	/** Find all tiles of given type */
	TArray<AM3TileActor*>FindTilesofType(int32 TileTypeID) const;

	/** Execute the result of one or more matches. It is possible, with multiple matches, to have more then one tile type in the array */
	void ExecuteMatch(const TArray<AM3TileActor*>& MatchingTiles);

	void OnTileWasSelected(AM3TileActor* NewSelectedTile);

	bool IsUnWinnable();

	/** Estableshes the most recent move type for the specified player */
	void SetLastMove(EMatch3MoveType::Type MoveType);

	EMatch3MoveType::Type GetLastMove();

	/** Gives points per tile based on move type. Default is 100 */
	UFUNCTION(BlueprintNativeEvent, Category = "Game")
	int32 GetScoreMultiplierForMove(EMatch3MoveType::Type LastMoveType);
	virtual int32 GetScoreMultiplierForMove_Implementation(EMatch3MoveType::Type LastMoveType);

	AM3TileActor* GetCurrentlySelectedTile() const { return CurrentlySelectedTile; };

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tile")
	AM3TileActor* CurrentlySelectedTile;

private:
	/** Array of tiles found in the most recent call to IsMoveLegal */
	TArray<AM3TileActor*> LastLegalMatch;

	/** Tiles that are currently falling */
	TArray<AM3TileActor*> FallingTiles;

	/** Tiles that are currentle swapping position with each other.Sholud be exactly two of them, or zero */
	TArray<AM3TileActor*> SwappingTiles;

	/** After spawning new tiles, wich tiles to check for automatic matches */
	TArray<AM3TileActor*> TilesToCheck;

	/** Tiles that are currently reacting to being matches */
	TArray<AM3TileActor*> TilesBeingDestroyed;

	/** The type of move last executed by a given player */
	TMap <class AM3PlayerController*, EMatch3MoveType::Type> LastMove;

	/** Indicates that we are waiting to complete a swap move. When SwappingTiles is populated by 2 tiles, we are done */
	uint32 bPendingSwapMove : 1;

	uint32 bPendingSwapMoveSuccess : 1;

};
