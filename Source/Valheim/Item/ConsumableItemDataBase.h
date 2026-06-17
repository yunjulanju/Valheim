// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemDataBase.h"
#include "ConsumableItemDataBase.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UConsumableItemDataBase : public UItemDataBase
{
	GENERATED_BODY()

public:
	virtual void Use(AArcher* User) override;
	
};
