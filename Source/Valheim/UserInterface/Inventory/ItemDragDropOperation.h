// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
class UItemDataBase;
class UInventoryComponent;
UCLASS()
class VALHEIM_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	UItemDataBase* SourceItem;

	UPROPERTY(VisibleAnywhere)
	UInventoryComponent* SourceInventory;
};
