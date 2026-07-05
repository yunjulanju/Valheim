// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStruct.h"
#include "InventoryComponent.generated.h"

class UItemSubsystem;

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	NoItemAdded UMETA(DisplayName = "No Item added"),
	PartialItemAdded UMETA(DisplayName = "Partial amount of Item added"),
	AllItemAdded UMETA(DisplayName = "All of Item added")
};

USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

	FItemAddResult()
		: ActualAmountAdded(0)
		, OperationResult(EItemAddResult::NoItemAdded)
	{
	}

	UPROPERTY(BlueprintReadOnly)
	int32 ActualAmountAdded;

	UPROPERTY(BlueprintReadOnly)
	EItemAddResult OperationResult;

	static FItemAddResult AddedNone()
	{
		FItemAddResult AddedNoneResult;
		AddedNoneResult.ActualAmountAdded = 0;
		AddedNoneResult.OperationResult = EItemAddResult::NoItemAdded;
		return AddedNoneResult;
	}

	static FItemAddResult AddedPartial(const int32 PartialAmountAdded)
	{
		FItemAddResult PartialAddedResult;
		PartialAddedResult.ActualAmountAdded = PartialAmountAdded;
		PartialAddedResult.OperationResult = EItemAddResult::PartialItemAdded;
		return PartialAddedResult;
	}

	static FItemAddResult AddedAll(const int32 AmountAdded)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.OperationResult = EItemAddResult::AllItemAdded;
		return AddedAllResult;
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VALHEIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	FOnInventoryUpdated OnInventoryUpdated;

	virtual void BeginPlay() override;

	// ===================== 조회 (Query) =====================

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryItemInstance GetInventoryItem(int32 InventoryIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryItemInstance GetHotbarItem(int32 HotbarIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventoryItemInstance> GetInventoryContents() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetSlotsCapacity() const
	{
		return InventorySlotsCapacity;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity)
	{
		InventorySlotsCapacity = NewSlotsCapacity;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetHotbarSlotsCapacity() const
	{
		return HotBarSlotsCapacity;
	}

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetUsedSlotCount() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindInventoryIndexByID(FName ItemID) const;

	// 구 FindNextPartialStack: 아직 다 안 찬 스택의 인덱스 (없으면 INDEX_NONE)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindNextPartialStackIndex(FName ItemID) const;

	// ===================== 추가 / 제거 (Mutation) =====================
	//
	// 아래 공개 함수들의 공통 규칙:
	// - 이미 Authority(서버) 상태에서 호출되면: RPC를 거치지 않고 대응하는
	//   _Internal 함수를 그 자리에서 직접 호출하여 실제 처리 결과를 즉시 반환합니다.
	//
	// - 클라이언트에서 호출되면: Server RPC로 요청만 보내고,
	//   실제 결과는 리플리케이션 + OnInventoryUpdated를 통해 반영됩니다.
	//   반환값은 placeholder입니다.

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult HandleAddItem(FName ItemID, int32 RequestedAmount);

	UFUNCTION(Server, Reliable)
	void Server_HandleAddItem(FName ItemID, int32 RequestedAmount);
	void Server_HandleAddItem_Implementation(FName ItemID, int32 RequestedAmount);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveAmountOfItem(int32 InventoryIndex, int32 DesiredAmountToRemove);

	UFUNCTION(Server, Reliable)
	void ServerRemoveAmountOfItem(int32 InventoryIndex, int32 DesiredAmountToRemove);
	void ServerRemoveAmountOfItem_Implementation(int32 InventoryIndex, int32 DesiredAmountToRemove);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveAmountOfHotbarItem(int32 HotbarIndex, int32 DesiredAmountToRemove);

	UFUNCTION(Server, Reliable)
	void ServerRemoveAmountOfHotbarItem(int32 HotbarIndex, int32 DesiredAmountToRemove);
	void ServerRemoveAmountOfHotbarItem_Implementation(int32 HotbarIndex, int32 DesiredAmountToRemove);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAtIndex(int32 InventoryIndex);

	UFUNCTION(Server, Reliable)
	void ServerRemoveItemAtIndex(int32 InventoryIndex);
	void ServerRemoveItemAtIndex_Implementation(int32 InventoryIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SplitExistingStack(int32 InventoryIndex, int32 AmountToSplit);

	UFUNCTION(Server, Reliable)
	void ServerSplitExistingStack(int32 InventoryIndex, int32 AmountToSplit);
	void ServerSplitExistingStack_Implementation(int32 InventoryIndex, int32 AmountToSplit);

	// ===================== 이동 (Move) =====================

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveInventoryItem(int32 FromIndex, int32 ToIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveInventoryItem(int32 FromIndex, int32 ToIndex);
	void ServerMoveInventoryItem_Implementation(int32 FromIndex, int32 ToIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItemToHotbar(int32 InventoryIndex, int32 HotbarIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveItemToHotbar(int32 InventoryIndex, int32 HotbarIndex);
	void ServerMoveItemToHotbar_Implementation(int32 InventoryIndex, int32 HotbarIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex);

	UFUNCTION(Server, Reliable)
	void ServerMoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex);
	void ServerMoveItemFromHotbarToInventorySlot_Implementation(int32 HotbarIndex, int32 TargetInventoryIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapHotbarSlots(int32 HotbarIndexA, int32 HotbarIndexB);

	UFUNCTION(Server, Reliable)
	void ServerSwapHotbarSlots(int32 HotbarIndexA, int32 HotbarIndexB);
	void ServerSwapHotbarSlots_Implementation(int32 HotbarIndexA, int32 HotbarIndexB);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UItemSubsystem* GetItemSubsystem() const;

	FItemAddResult HandleNoneStackableItems(FName ItemID, int32 RequestedAddAmount);
	int32 HandleStackableItems(FName ItemID, int32 RequestedAddAmount);

	// 서버 로직 전용
	int32 CalculateAmountForFullStack(
		int32 CurrentQuantity,
		int32 MaxStackSize,
		int32 InitialRequestedAmount) const;

	// 빈 슬롯에 새 스택 생성
	void AddNewItemAtIndex(int32 SlotIndex, FName ItemID, int32 AmountToAdd);

	// FFastArraySerializerItem의 ReplicationID 등은 건드리지 않고 ItemID/Quantity만 교환
	static void SwapItemContents(FInventoryItemInstance& A, FInventoryItemInstance& B);

	// ---- Internal 실제 처리 함수들 ----
	FItemAddResult HandleAddItem_Internal(FName ItemID, int32 RequestedAmount);
	int32 RemoveAmountOfItem_Internal(int32 InventoryIndex, int32 DesiredAmountToRemove);
	int32 RemoveAmountOfHotbarItem_Internal(int32 HotbarIndex, int32 DesiredAmountToRemove);
	void RemoveItemAtIndex_Internal(int32 InventoryIndex);
	bool SplitExistingStack_Internal(int32 InventoryIndex, int32 AmountToSplit);
	bool MoveInventoryItem_Internal(int32 FromIndex, int32 ToIndex);
	bool MoveItemToHotbar_Internal(int32 InventoryIndex, int32 HotbarIndex);
	bool MoveItemFromHotbarToInventorySlot_Internal(int32 HotbarIndex, int32 TargetInventoryIndex);
	void SwapHotbarSlots_Internal(int32 HotbarIndexA, int32 HotbarIndexB);

protected:
	// Property ///////////////////////////////////////////////

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 InventorySlotsCapacity = 20;

	UPROPERTY(Replicated)
	FInventoryList InventoryContents;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 HotBarSlotsCapacity = 9;

	UPROPERTY(Replicated)
	FInventoryList HotbarContents;
};