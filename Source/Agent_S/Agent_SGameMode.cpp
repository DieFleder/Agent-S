// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Agent_SGameMode.h"
#include "Agent_SCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAgent_SGameMode::AAgent_SGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
