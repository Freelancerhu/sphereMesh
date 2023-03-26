// Copyright Epic Games, Inc. All Rights Reserved.

#include "space_projectGameMode.h"
#include "space_projectPlayerController.h"
#include "space_projectCharacter.h"
#include "UObject/ConstructorHelpers.h"

Aspace_projectGameMode::Aspace_projectGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = Aspace_projectPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}