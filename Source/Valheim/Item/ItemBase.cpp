// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Components/BoxComponent.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AItemBase::AItemBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetEnableGravity(true);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetIsReplicated(true); //메시 동기화

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(ItemMesh);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && DesiredItemAsset)
	{
		InitializeItem(Quantity);
	}
}

void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemBase::InitializeItem(const int32 InQuantity)
{
	if (!DesiredItemAsset)
	{
		return;
	}

	ItemID = DesiredItemAsset->ItemID;
	Quantity = FMath::Max(InQuantity, 0);
	CurrentMeshAsset = DesiredItemAsset->AssetData.ItemMesh;

	// 서버(리슨서버 포함)는 OnRep이 호출되지 않으므로 직접 적용
	ItemMesh->SetStaticMesh(CurrentMeshAsset);
}

void AItemBase::InitiallizeDrop(FName InItemID, const int32 InQuantity)
{
	ItemID = InItemID;
	Quantity = FMath::Max(InQuantity, 0);

	UItemPrimaryDataAsset* ItemData = nullptr;
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (const UItemSubsystem* ItemSubsystem = GI->GetSubsystem<UItemSubsystem>())
			{
				ItemSubsystem->GetItemData(InItemID, ItemData);
			}
		}
	}

	if (ItemData)
	{
		CurrentMeshAsset = ItemData->AssetData.ItemMesh;
		ItemMesh->SetStaticMesh(CurrentMeshAsset);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InitiallizeDrop: ItemID %s not found in ItemSubsystem"), *InItemID.ToString());
	}
}

void AItemBase::OnRep_CurrentMeshAsset()
{
	ItemMesh->SetStaticMesh(CurrentMeshAsset);
}

bool AItemBase::TryAddToInventory(AArcher* Taker)
{
	if (!Taker || ItemID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("AItemBase::TryAddToInventory: invalid Taker or ItemID"));
		return false;
	}

	UInventoryComponent* PlayerInventory = Taker->GetInventory();
	if (!PlayerInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("AItemBase::TryAddToInventory: PlayerInventory NULL"));
		return false;
	}

	const FItemAddResult AddResult = PlayerInventory->HandleAddItem(ItemID, Quantity);

	switch (AddResult.OperationResult)
	{
	case EItemAddResult::NoItemAdded:
		return false;
	case EItemAddResult::PartialItemAdded:
		// 일부만 들어간 경우: 남은 수량만 픽업에 남겨두고 파괴하지 않음
		Quantity -= AddResult.ActualAmountAdded;
		return false;
	case EItemAddResult::AllItemAdded:
		return true;
	}
	return false;
}

void AItemBase::TakePickUp(const AArcher* Taker)
{
	if (IsPendingKillPending())
	{
		return;
	}

	if (TryAddToInventory(const_cast<AArcher*>(Taker)))
	{
		Destroy();
	}
}

void AItemBase::Interact(APawn* Interactor)
{
	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("AItemBase::Interact !Interactor"));
		return;
	}
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AItemBase::Interact !HasAuthority()"));
		return;
	}
	AArcher* Taker = Cast<AArcher>(Interactor);
	if (!Taker)
	{
		return;
	}

	if (IsPendingKillPending())
	{
		return;
	}

	if (TryAddToInventory(Taker))
	{
		Destroy();
	}
}

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AItemBase, ItemID);
	DOREPLIFETIME(AItemBase, Quantity);
	DOREPLIFETIME(AItemBase, CurrentMeshAsset);
}