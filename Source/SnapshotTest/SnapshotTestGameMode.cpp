// Copyright Epic Games, Inc. All Rights Reserved.

#include "SnapshotTestGameMode.h"
#include "SnapshotTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASnapshotTestGameMode::ASnapshotTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
