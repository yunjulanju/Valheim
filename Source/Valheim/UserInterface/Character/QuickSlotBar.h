// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuickSlotBar.generated.h"

class AArcher;
class UInventoryComponent;
class UHorizontalBox;
class UHotBarSlot;

UCLASS()
class VALHEIM_API UQuickSlotBar : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void RefreshQuickSlots();

	void UpdateHighlight();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> QuickSlotBox;

	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	TSubclassOf<UHotBarSlot> HotbarSlotClass;

	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	int32 QuickSlotCount = 9;

protected:
	UPROPERTY()
	TObjectPtr<AArcher> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryReference;

	UPROPERTY()
	TArray<TObjectPtr<UHotBarSlot>> QuickSlots;
};