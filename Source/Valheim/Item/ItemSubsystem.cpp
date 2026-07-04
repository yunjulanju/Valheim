// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemSubsystem.h"
#include "Core/ValheimGI.h"
#include "Item/ItemRegistry.h"
#include "Data/ItemPrimaryDataAsset.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UValheimGI* GI = Cast<UValheimGI>(GetGameInstance()))
	{
		ItemRegistry = GI->ItemRegistry;
	}

	BuildItemCache();
}

void UItemSubsystem::BuildItemCache()
{
	ItemDataCache.Empty();

	if (!ItemRegistry)
	{
		UE_LOG(LogTemp, Warning, TEXT("UItemSubsystem: ItemRegistry Null"));
		return;
	}

	for (UItemPrimaryDataAsset* Item : ItemRegistry->AllItems)
	{
		if (Item)
		{
			ItemDataCache.Add(Item->ItemID, Item);
		}
	}
}

bool UItemSubsystem::GetItemData(FName ItemID, UItemPrimaryDataAsset*& OutData) const
{
	if (const TObjectPtr<UItemPrimaryDataAsset>* Found = ItemDataCache.Find(ItemID))
	{
		OutData = *Found;
		return true;
	}
	return false;
}

bool UItemSubsystem::DoesItemExist(FName ItemID) const
{
	return ItemDataCache.Contains(ItemID);
}