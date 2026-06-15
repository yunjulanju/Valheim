// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemSlot.generated.h"

/**
 * 
 */
class UItemDataBase;
class UDragItemVisual;
class UInventoryToolTip;
class UBorder;
class UImage;
class UTextBlock;
UCLASS()
class VALHEIM_API UInventoryItemSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetItemReference(UItemDataBase* ItemIn) { ItemReference = ItemIn; }
	FORCEINLINE UItemDataBase* GetItemReference() const { return ItemReference; }

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	//virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDragItemVisual> DragItemVisualClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryToolTip> ToolTipClass;

	UPROPERTY(VisibleAnywhere)
	UItemDataBase* ItemReference;

	UPROPERTY(meta = (BindWidget))
	UBorder* ItemBorder;
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemQuantity;
};
