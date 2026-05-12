// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValheimGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AValheimGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AValheimGameMode();
};



