#include "Data/ItemDataStruct.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "Item/ItemSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/GameInstance.h"


bool FInventoryItemInstance::IsStackable() const
{
	return CachedItemData && CachedItemData->NumericData.bIsStackable;
}

bool FInventoryItemInstance::IsFullStack() const
{
	return CachedItemData && Quantity >= CachedItemData->NumericData.MaxStackSize;
}


void FInventoryItemInstance::PostReplicatedAdd(const FInventoryList& InArraySerializer)
{
	if (!InArraySerializer.OwningComponent)
	{
		return;
	}

	if (const UGameInstance* GI = InArraySerializer.OwningComponent->GetWorld()->GetGameInstance())
	{
		if (const UItemSubsystem* ItemSubsystem = GI->GetSubsystem<UItemSubsystem>())
		{
			UItemPrimaryDataAsset* Found = nullptr;
			/*if (ItemSubsystem->GetItemData(ItemID, Found))
			{
				CachedItemData = Found;
			}*/
		}
	}
}

void FInventoryItemInstance::PostReplicatedChange(const FInventoryList& InArraySerializer)
{
	PostReplicatedAdd(InArraySerializer);
}

void FInventoryItemInstance::PreReplicatedRemove(const FInventoryList& InArraySerializer)
{
	
}