// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "Sword.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API ASword : public AItemBase
{
	GENERATED_BODY()

public:
	void PickItem();
	
};
