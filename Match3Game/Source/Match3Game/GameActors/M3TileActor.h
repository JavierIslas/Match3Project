// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteActor.h"
#include "Engine/Classes/Engine/EngineTypes.h"
#include "M3TileActor.generated.h"


class USoundWave;

UENUM()
namespace ETileState
{
	enum Type
	{
		ETS_Normal,
		ETS_Falling,
		ETS_PendingDelete
	};
}

UENUM(BlueprintType)
namespace EMatch3MoveType
{
	enum Type
	{
		MT_None,
		MT_Failure,
		MT_Standard,
		MT_MoreTiles,
		MT_Combo
	};
}

USTRUCT(BlueprintType)
struct FTileAbilities
{
	GENERATED_USTRUCT_BODY();

	bool CanExplode() { return bExplodes; }
	bool CanSwap() { return (!bPreventSwapping && !bExplodes); }

protected:
	/** Tile explodes when selected (change this!) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bExplodes : 1;

	/** Tile can't be selected as part of a normal swapping move. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bPreventSwapping : 1;

public:
	/** Power rating of a bomb. What this means is determined in GameMode code, and can consider what kind of bomb this is. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BombPower = 0;

	// Replace this when bitfields can have default member initializers
	FTileAbilities() : bExplodes(0), bPreventSwapping(0) {}
};

UCLASS()
class MATCH3GAME_API AM3TileActor : public APaperSpriteActor
{
	GENERATED_BODY()

public:
	AM3TileActor();

	void BeginPlay() override;

	UFUNCTION()
	void TilePress(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

	UFUNCTION()
	void TileEnter(ETouchIndex::Type FingerIndex, AActor* TouchedActor);

	/**Mouse for testing on PC */
	UFUNCTION()
	void TilePress_Mouse(AActor* CleckedActor, FKey ButtonClicked);

	UFUNCTION()
	void TileEnter_Mouse(AActor* MousedOverActor);

	/**For Blueprint Implementation to play effects and particle effects */
	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Event")
	void PlaySelectionEffect(bool bTurnEffectOn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Event")
	void StartFallingEffect();

	UFUNCTION(BlueprintImplementableEvent, Category = "Special Game Event")
	void StopFallingEffect();

	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Event")
	void SetTileMaterial(class UMaterialInstanceConstant* TileMaterial);
	virtual void SetTileMaterial_Implementation(class UMaterialInstanceConstant* TileMaterial);

	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Event")
	void OnMatched(EMatch3MoveType::Type MoveType);
	virtual void OnMatched_Implementation(EMatch3MoveType::Type MoveType);

	UFUNCTION(BlueprintNativeEvent, Category = "Special Game Event")
	void OnSwapMove(AM3TileActor* OtherTile, bool bMoveWillSucced);
	virtual void OnSwapMove_Implementation(AM3TileActor* OtherTile, bool bMoveWillSucced);

	void StartFalling(bool bUseCurrentWorldLocation = false);

	USoundWave* GetMatchSound();

	UFUNCTION()
	void TickFalling();

	void StopFalling();

	void SetTableAddress(int32 NewLocation);

	int32 GetTableAddress() const;

	UPROPERTY(BlueprintReadOnly)
	int32 TileTypeID;

	UPROPERTY()
	TEnumAsByte<ETileState::Type> TileState;

	UPROPERTY(BlueprintReadOnly)
	FTileAbilities Abilities;

protected:

	float TotalFallingTime;

	float FallingStartTime;

	FVector FallingStartLocation;

	FVector FallingEndLocation;

	FTimerHandle TimerHandle_TickFalling;

	/** Location in the table as a 1D value, the table can translate it to a position*/
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	int32 TableAddress;

	/** Location in the table where will land as a 1D value, the table can translate it to a position. Used while falling.*/
	int32 LandingAddress;

	/**Table where the Tile is */
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	class AM3TableActor* TableOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio", Meta = (BlueprintProtected = True))
	USoundWave* MatchSound;
};
