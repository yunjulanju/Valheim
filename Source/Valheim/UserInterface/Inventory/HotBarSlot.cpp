
// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/HotBarSlot.h"

void UHotBarSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHotBarSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

bool UHotBarSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
