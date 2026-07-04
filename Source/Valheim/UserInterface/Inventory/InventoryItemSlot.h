// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "InventoryItemSlot.generated.h"

/**
 *
 */
class UItemPrimaryDataAsset;
class UInventoryComponent;
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

	FORCEINLINE void SetItem(const FInventoryItemInstance& InItem) { CurrentItem = InItem; }
	FORCEINLINE const FInventoryItemInstance& GetItem() const { return CurrentItem; }

	FORCEINLINE void SetOwningInventory(UInventoryComponent* InInventory) { OwningInventory = InInventory; }
	FORCEINLINE UInventoryComponent* GetOwningInventory() const { return OwningInventory; }
	FORCEINLINE void SetOwningInventoryIndex(int32 InIndex) { OwningInventoryIndex = InIndex; }
	FORCEINLINE int32 GetOwningInventoryIndex() const { return OwningInventoryIndex; }

	void RefreshSlot();

	UItemPrimaryDataAsset* GetDisplayItemData() const;

	bool bShowToolTip = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SlotIndex = 0;

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDragItemVisual> DragItemVisualClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryToolTip> ToolTipClass;

	UPROPERTY(VisibleAnywhere)
	FInventoryItemInstance CurrentItem;

	UPROPERTY()
	UInventoryComponent* OwningInventory = nullptr;

	int32 OwningInventoryIndex = -1;

	UPROPERTY(meta = (BindWidget))
	UBorder* ItemBorder;
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemQuantity;
};