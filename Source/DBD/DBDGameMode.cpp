// Copyright Epic Games, Inc. All Rights Reserved.

#include "DBDGameMode.h"
#include "DBDCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADBDGameMode::ADBDGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
