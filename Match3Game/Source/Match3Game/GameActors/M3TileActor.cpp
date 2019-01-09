// Fill out your copyright notice in the Description page of Project Settings.

#include "M3TileActor.h"
#include "PaperSprite.h"
#include "M3TableActor.h"
#include "../GameFramework/M3GameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AM3TileActor::AM3TileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	//It use a Scene Component to base the tile on
	if (RootComponent)
	{
		RootComponent->SetMobility(EComponentMobility::Movable);
	}
}

void AM3TileActor::BeginPlay()
{
	Super::BeginPlay();

	TableOwner = Cast<AM3TableActor>(GetOwner());

	//Set up to handle touch events
	OnInputTouchBegin.AddUniqueDynamic(this, &AM3TileActor::TilePress);
	OnInputTouchEnter.AddUniqueDynamic(this, &AM3TileActor::TileEnter);
}

void AM3TileActor::TilePress(ETouchIndex::Type FingerIndex, AActor * TouchedActor)
{
	//If (the tile is) clicked or touched
	if (!UGameplayStatics::IsGamePaused(this) && TableOwner)
	{
		TableOwner->OnTileWasSelected(this);
	}
}

void AM3TileActor::TileEnter(ETouchIndex::Type FingerIndex, AActor * TouchedActor)
{
	//Moved into the tile's space while we had a differnt tile selected. This is the same as pressing the tile dierctly
	//Nore that need to make sure it's a different actual tile (i.e. not null) because deselecting a tile by touching it twice will then trigger the tileEnter event and re-select it
	if (!UGameplayStatics::IsGamePaused(this) && TableOwner)
	{
		AM3TileActor* CurrentlySelectedTile = TableOwner->GetCurrentlySelectedTile();
		if (CurrentlySelectedTile && (CurrentlySelectedTile != this))
		{
			TilePress(FingerIndex, TouchedActor);
		}
	}
}

void AM3TileActor::TilePress_Mouse(AActor * ClickedActor, FKey ButtonClicked)
{
	TilePress(ETouchIndex::Touch1, ClickedActor);
}

void AM3TileActor::TileEnter_Mouse(AActor * MousedOverActor)
{
	//This is meat to simulate fingerswipng, so ignore if the mouse isn't cliecked
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PC->IsInputKeyDown(EKeys::LeftMouseButton))
		{
			TileEnter(ETouchIndex::Touch1, MousedOverActor);
		}
	}
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
