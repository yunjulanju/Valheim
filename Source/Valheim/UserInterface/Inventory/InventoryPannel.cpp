// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryPannel.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Item/ItemDataBase.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"
#include "ItemDragDropOperation.h"

void UInventoryPannel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter = Cast<AArcher>(GetOwningPlayerPawn());
	if (PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UInventoryPannel::RefreshInventory);
			SetInfoText();
		}
	}

}

void UInventoryPannel::SetInfoText() const
{
	CapacityInfo->SetText(FText::Format(FText::FromString("{0}/{1}"), InventoryReference->GetInventoryContents().Num(), InventoryReference->GetSlotsCapacity()));
}

void UInventoryPannel::RefreshInventory()
{
	if (InventoryReference && InventorySlotClass)
	{
		InventoryPannel->ClearChildren();
		for (UItemDataBase* const& InventoryItem : InventoryReference->GetInventoryContents())
		{
			if (InventoryItem->Quantity <= 0) continue;
			//UE_LOG(LogTemp, Warning, TEXT("InventoryReference->GetInventoryContents()"))
			UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);
			ItemSlot->SetItemReference(InventoryItem);

			InventoryPannel->AddChildToWrapBox(ItemSlot);
		}
		SetInfoText();
	}
}

bool UInventoryPannel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	if (ItemDragDrop->SourceItem && InventoryReference)
	{
		return true;
	}

	return false;
}
