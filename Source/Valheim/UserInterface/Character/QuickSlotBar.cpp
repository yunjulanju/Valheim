// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/Character/QuickSlotBar.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "UserInterface/Inventory/HotBarSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"

void UQuickSlotBar::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UQuickSlotBar::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar]GetOwningPlayerPawn() returned NULL! Init Failed"));
	}

	PlayerCharacter = Cast<AArcher>(OwningPawn);
	if (PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.RemoveAll(this);
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UQuickSlotBar::RefreshQuickSlots);
			UE_LOG(LogTemp, Log, TEXT("[QuickSlotBar]InventoryComponent Binding Success!"));
		}

		PlayerCharacter->OnActiveHotbarChanged.RemoveAll(this);
		PlayerCharacter->OnActiveHotbarChanged.AddUObject(this, &UQuickSlotBar::UpdateHighlight);
	}
	else if (OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] Pawn exists but Cast to AArcher failed! (Pawn Name: %s)"),  *OwningPawn->GetName());
	}

	RefreshQuickSlots();
}

void UQuickSlotBar::NativeDestruct()
{
	if (InventoryReference)
	{
		InventoryReference->OnInventoryUpdated.RemoveAll(this);
		InventoryReference = nullptr;
	}
	if (PlayerCharacter)
	{
		PlayerCharacter->OnActiveHotbarChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UQuickSlotBar::RefreshQuickSlots()
{

	if (!InventoryReference)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] RefreshQuickSlots Failed: InventoryReference is NULL."));
		return;
	}
	if (!HotbarSlotClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] RefreshQuickSlots Failed: HotbarSlotClass is NULL."));
		return;
	}
	if (!QuickSlotBox)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] RefreshQuickSlots Failed: QuickSlotBox is NULL."));
		return;
	}

	QuickSlotBox->ClearChildren();
	QuickSlots.Empty();

	int32 CreatedCount = 0;
	for (int32 i = 0; i < QuickSlotCount; i++)
	{
		UHotBarSlot* QuickSlot = CreateWidget<UHotBarSlot>(this, HotbarSlotClass);
		if (!QuickSlot)
		{
			UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] slot Widget (%d) Create Failed!"), i);
			continue;
		}

		QuickSlot->SlotIndex = i;
		if (QuickSlot->HotKeyNumber)
		{
			QuickSlot->HotKeyNumber->SetText(FText::AsNumber(i + 1));
		}

		const FInventoryItemInstance HotbarItem = InventoryReference->GetHotbarItem(i);

		QuickSlot->SetOwningInventory(InventoryReference);
		QuickSlot->SetItem(HotbarItem);
		QuickSlot->bShowToolTip = HotbarItem.IsValidItem();
		QuickSlot->RefreshSlot();

		QuickSlotBox->AddChildToHorizontalBox(QuickSlot);
		QuickSlots.Add(QuickSlot);
		CreatedCount++;
	}

	UpdateHighlight();
}

void UQuickSlotBar::UpdateHighlight()
{
	if (!PlayerCharacter)
	{
		return;
	}

	const int32 ActiveIndex = PlayerCharacter->GetActiveHotbarIndex();
	for (int32 i = 0; i < QuickSlots.Num(); i++)
	{
		if (QuickSlots[i])
		{
			QuickSlots[i]->SetHighlighted(i == ActiveIndex);
		}
	}
}