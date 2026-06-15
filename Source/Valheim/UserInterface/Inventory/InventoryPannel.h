// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryPannel.generated.h"

/**
 * 
 */
class UWrapBox;
class UTextBlock;
class AArcher;
class UInventoryComponent;
class UInventoryItemSlot;
UCLASS()
class VALHEIM_API UInventoryPannel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void RefreshInventory();

	UPROPERTY(meta = (BindWidget))
	UWrapBox* InventoryPannel;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CapacityInfo;
	UPROPERTY()
	AArcher* PlayerCharacter;
	UPROPERTY()
	UInventoryComponent* InventoryReference;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryItemSlot> InventorySlotClass;

protected:
	void SetInfoText() const;
	virtual void NativeOnInitialized() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
