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
class UHorizontalBox;
class UInventoryItemSlot;
class UHotBarSlot;
UCLASS()
class VALHEIM_API UInventoryPannel : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void RefreshInventory();

	UFUNCTION()
	void RefreshHotbar();

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* HotbarBox;

	UPROPERTY()
	TArray<UHotBarSlot*> HotbarSlots;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHotBarSlot> HotbarSlotClass;

	UPROPERTY(EditAnywhere)
	int32 HotbarSlotCount = 9;

	UPROPERTY(EditAnywhere)
	int32 SlotCount;

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
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};