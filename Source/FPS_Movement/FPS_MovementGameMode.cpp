// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_MovementGameMode.h"
#include "FPS_MovementHUD.h"
#include "FPS_MovementCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPS_MovementGameMode::AFPS_MovementGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPS_MovementHUD::StaticClass();
}
