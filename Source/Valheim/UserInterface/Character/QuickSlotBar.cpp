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

	// 컴파일러 에러 방지를 위해 로그 내부 한글 및 제어 문자 제거
	FString NetModeStr = (GetWorld() && GetWorld()->GetNetMode() == NM_Client) ? TEXT("Client") : TEXT("Server_Host");
	UE_LOG(LogTemp, Warning, TEXT("[QuickSlotBar] NativeConstruct Called on [%s]"), *NetModeStr);

	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] [%s] GetOwningPlayerPawn() returned NULL! Init Failed"), *NetModeStr);
	}

	PlayerCharacter = Cast<AArcher>(OwningPawn);
	if (PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.RemoveAll(this);
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UQuickSlotBar::RefreshQuickSlots);
			UE_LOG(LogTemp, Log, TEXT("[QuickSlotBar] [%s] InventoryComponent Binding Success!"), *NetModeStr);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] [%s] Character exists but InventoryComponent is NULL!"), *NetModeStr);
		}
	}
	else if (OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("[QuickSlotBar] [%s] Pawn exists but Cast to AArcher failed! (Pawn Name: %s)"), *NetModeStr, *OwningPawn->GetName());
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

}