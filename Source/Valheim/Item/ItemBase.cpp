// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "ItemDataBase.h"
#include <Character/Archer.h>
#include "Inventory/InventoryComponent.h"

// Sets default values
AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(ItemMesh);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	//초기화
	InitializeItem(Quantity);
}

//에디터에서 값을 바꿀 때 바로 적용할 수 있도록 하는 함수 (없어도 될듯)
void AItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FString ChangedProPertyStr = PropertyChangedEvent.Property->GetName();
		const FName ChangedPropertyName = FName(ChangedProPertyStr);
		
		if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(AItemBase, DesiredItemID))
		{
			if (ItemDataTable)
			{
				if (const FItemBaseRow* ItemData = ItemDataTable->FindRow<FItemBaseRow>(DesiredItemID, DesiredItemID.ToString()))
				{
					ItemMesh->SetStaticMesh(ItemData->AssetData.ItemMesh);
				}
			}
		}
	}
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemBase::InitializeItem(const int32 InQuantity)
{
	if (ItemDataTable && !DesiredItemID.IsNone())
	{
		const FItemBaseRow* ItemData = ItemDataTable->FindRow<FItemBaseRow>(DesiredItemID, DesiredItemID.ToString());

		ItemReference = NewObject<UItemDataBase>(this, UItemDataBase::StaticClass());

		ItemReference->ItemID = ItemData->ItemID;
		ItemReference->ItemCategory = ItemData->ItemCategory;
		ItemReference->AssetData = ItemData->AssetData;
		ItemReference->NumericData = ItemData->NumericData;
		ItemReference->TextData = ItemData->TextData;

		if (InQuantity <= 0)
		{
			ItemReference->SetQuantity(0);
		}
		else
		{
			ItemReference->SetQuantity(InQuantity);
		}

		ItemMesh->SetStaticMesh(ItemData->AssetData.ItemMesh);
	}
}

void AItemBase::InitiallizeDrop(UItemDataBase* ItemToDrop, const int32 InQuantity)
{

	ItemReference = ItemToDrop->CreateItemCopy();
	ItemReference->bIsPickup = true;

	if (InQuantity <= 0)
	{ 
		ItemReference->SetQuantity(0);
	}
	else
	{
		ItemReference->SetQuantity(InQuantity);
	}
	ItemReference->OwningInventory = nullptr;
	ItemMesh->SetStaticMesh(ItemToDrop->AssetData.ItemMesh);
}

void AItemBase::TakePickUp(const AArcher* Taker)
{
	if (!IsPendingKillPending())
	{
		if (ItemReference)
		{
			//인벤토리
			if(UInventoryComponent* PlayerInventory = Taker->GetInventory())
			{

				ItemReference->bIsPickup = true;
				const FItemAddResult AddResult = PlayerInventory->HandleAddItem(ItemReference);

				switch (AddResult.OperationResult)
				{
				case EItemAddResult::NoItemAdded:
					break;
				case EItemAddResult::PartialItemAdded:
					break;
				case EItemAddResult::AllItemAdded:
					Destroy();
					break;
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("PlayerInventory NO"))
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ItemReference NO"))
		}
	}	
}