// Fill out your copyright notice in the Description page of Project Settings.

#include "M3TileActor.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "M3TableActor.h"
#include "../GameFramework/M3GameMode.h"
#include "TimerManager.h"
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

/**Check when everything fails, TODO change to TSubClassof<UMaterialInterface> <---- DOWNCAST*/
void AM3TileActor::SetTileMaterial_Implementation(class UMaterialInterface * TileMaterial)
{
	GetRenderComponent()->SetMaterial(0, TileMaterial);
}

void AM3TileActor::OnMatched_Implementation(EMatch3MoveType::Type MoveType)
{
	TableOwner->OnTileFinishedMatching(this);
}

void AM3TileActor::OnSwapMove_Implementation(AM3TileActor * OtherTile, bool bMoveWillSucced)
{
	TableOwner->OnSwapDisplayFinished(this);
}

void AM3TileActor::StartFalling(bool bUseCurrentWorldLocation)
{
	float FallDistance = 0;

	FallingStartTime = GetWorld()->GetTimeSeconds();
	FallingStartLocation = GetActorLocation();
	//Tiles, fall at a fixed rate of 120 FPS
	GetWorldTimerManager().SetTimer(TimerHandle_TickFalling, this, &AM3TileActor::TickFalling, 0.001f, true);
	check(TableOwner);
	
	if (!bUseCurrentWorldLocation)
	{
		//Fall from where we are on the table to wher we are supposed to beon the table
		int32 YOffset = 0;
		int32 HeightAboveBottom = 1;
		while (true)
		{
			++YOffset;
			if (TableOwner->GetTableAddressWithOffset(GetTableAddress(), 0, -YOffset, LandingAddress))
			{
				if (AM3TileActor* TileBelow = TableOwner->GetTileFromTableAddress(LandingAddress))
				{
					//We're not off the table, so check to see what is in this space and reack to it
					if (TileBelow->TileState == ETileState::ETS_Falling)
					{
						//This space contains a fallig thile, so contiue to fall through it, but note that the tile will land underneath us, so we need to leave a gap for it
						++HeightAboveBottom;
						continue;
					}
					else if (TileBelow->TileState == ETileState::ETS_PendingDelete)
					{
						//This space contains a tile that is about to be deleted. WE can fall through this space freely
						continue;
					}
				}
				else
				{
					//The space below is empty, but is on the table. We can fall through this space freely
					continue;
				}
			}
			//This space is off the table or contains a tile that is staying. Go back one space and stop
			YOffset -= HeightAboveBottom;
			TableOwner->GetTableAddressWithOffset(GetTableAddress(), 0, -YOffset, LandingAddress);
			break;
		}
		FallDistance = TableOwner->TileSize.Y * YOffset;
		FallingEndLocation = FallingStartLocation;
		FallingEndLocation.Z -= FallDistance;
	}
	else
	{
		//Fall from where we are physically to where we are supposed to be on the table
		LandingAddress = GetTableAddress();
		FallingEndLocation = TableOwner->GetLocationFromTableAddress(LandingAddress);
		FallDistance = FallingStartLocation.Z - FallingEndLocation.Z;
	}

	AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(this));
	TotalFallingTime = 0.0f;
	if (GM && (GM->TileMoveSpeed > 0.0f))
	{
		TotalFallingTime = FallDistance / GM->TileMoveSpeed;
	}
	if (TotalFallingTime <= 0.0f)
	{
		TotalFallingTime = 0.75f;
	}
	StartFallingEffect();
}

USoundWave * AM3TileActor::GetMatchSound()
{
	return MatchSound;
}

void AM3TileActor::TickFalling()
{
	AM3GameMode* GM = Cast<AM3GameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		check(TableOwner);
		check(TotalFallingTime > 0.0f);
		float FallCompleteFraction = (GetWorld()->GetTimeSeconds() - FallingStartTime) / TotalFallingTime;

		//Stop falling if we're at the final location. Otherwise, continue to move
		if (FallCompleteFraction >= 1.0f)
		{
			StopFalling();
		}
		else
		{
			FVector NewLocation = FMath::Lerp(FallingStartLocation, FallingEndLocation, FallCompleteFraction);
			SetActorLocation(NewLocation);
		}
	}
	else
	{
		//Error. Stop ticking this function. Move the tile to the final location
		StopFalling();
	}
}

void AM3TileActor::StopFalling()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TickFalling);
	SetActorLocation(FallingEndLocation);
	TableOwner->OnTileFinishedFalling(this, LandingAddress);
	StopFallingEffect();
}

void AM3TileActor::SetTableAddress(int32 NewLocation)
{
	TableAddress = NewLocation;
}

int32 AM3TileActor::GetTableAddress() const
{
	return TableAddress;
}
