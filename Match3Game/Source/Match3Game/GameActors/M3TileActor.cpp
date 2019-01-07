// Fill out your copyright notice in the Description page of Project Settings.

#include "M3TileActor.h"
#include "PaperSprite.h"

// Sets default values
AM3TileActor::AM3TileActor()
{

}

void AM3TileActor::BeginPlay()
{
}

void AM3TileActor::TilePress(ETouchIndex::Type FingerIndex, AActor * TouchedActor)
{
}

void AM3TileActor::TileEnter(ETouchIndex::Type FingerIndex, AActor * TouchedActor)
{
}

void AM3TileActor::TilePress_Mouse(AActor * CleckedActor, FKey ButtonClicked)
{
}

void AM3TileActor::TileEnter_Mouse(AActor * MousedOverActor)
{
}

void AM3TileActor::SetTileMaterial_Implementation(UMaterialInstanceConstant * TileMaterial)
{
}

void AM3TileActor::OnMatched_Implementation(EMatch3MoveType::Type MoveType)
{
}

void AM3TileActor::OnSwapMove_Implementation(AM3TileActor * OtherTile, bool bMoveWillSucced)
{
}

void AM3TileActor::StartFalling(bool bUseCurrentWorldLocation)
{
}

USoundWave * AM3TileActor::GetMatchSound()
{
	return nullptr;
}

void AM3TileActor::TickFalling()
{
}

void AM3TileActor::StopFalling()
{
}

void AM3TileActor::SetTableAddress(int32 NewLocation)
{
}

int32 AM3TileActor::GetTableAddress() const
{
	return int32();
}
