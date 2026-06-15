// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryToolTip.generated.h"

/**
 * 
 */
class UInventoryItemSlot;
class UTextBlock;
UCLASS()
class VALHEIM_API UInventoryToolTip : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	UInventoryItemSlot* InventorySlotBeingHovered;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCategory;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescription;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxStackSize;

protected:
	virtual void NativeConstruct() override;
	
};
