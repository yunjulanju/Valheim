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

	FItemAddResult() : ActualAmountAdded(0), OperationResult(EItemAddResult::NoItemAdded)
	{
	};

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

	// 핫바 슬롯이 가리키는 인벤토리 아이템을 반환 (핫바는 실체를 안 가지고 인덱스만 참조함)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryItemInstance GetHotbarItem(int32 HotbarIndex) const;

	// 핫바 슬롯이 인벤토리 몇 번을 가리키는지 (UI에서 "이 슬롯은 핫바에 등록됨" 표시할 때 사용)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetHotbarSlotInventoryIndex(int32 HotbarIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventoryItemInstance> GetInventoryContents() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetSlotsCapacity() const { return InventorySlotsCapacity; }

	// BeginPlay 이전(생성자 등)에만 호출할 것. BeginPlay에서 이 값으로 InventoryContents 크기를 잡음
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity) { InventorySlotsCapacity = NewSlotsCapacity; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetHotbarSlotsCapacity() const { return HotBarSlotsCapacity; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetUsedSlotCount() const;

	// 구 FindMatchingItem / FindNextItemByID 통합: 해당 ItemID를 가진 첫 슬롯의 인덱스 (없으면 INDEX_NONE)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindInventoryIndexByID(FName ItemID) const;

	// 구 FindNextPartialStack: 아직 다 안 찬 스택의 인덱스 (없으면 INDEX_NONE)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindNextPartialStackIndex(FName ItemID) const;

	// ===================== 추가 / 제거 (Mutation) =====================

	// 서버에서만 호출할 것. ItemID 기준으로 스택 가능 여부/최대치는 ItemSubsystem에서 직접 조회함
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult HandleAddItem(FName ItemID, int32 RequestedAmount);

	// 반환값 = 실제로 제거된 수량. 0이 되면 슬롯 자체를 비우고 핫바 참조도 정리함
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveAmountOfItem(int32 InventoryIndex, int32 DesiredAmountToRemove);

	// 수량과 무관하게 슬롯을 통째로 비움 (구 RemoveSingleInstanceOfItem)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAtIndex(int32 InventoryIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SplitExistingStack(int32 InventoryIndex, int32 AmountToSplit);

	// ===================== 이동 (Move) =====================

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveInventoryItem(int32 FromIndex, int32 ToIndex);

	// 주의: 이제 인벤토리에서 "빼내는" 게 아니라 핫바가 해당 인벤토리 슬롯을 "가리키게" 만듦
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItemToHotbar(int32 InventoryIndex, int32 HotbarIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex);

	// 두 핫바 슬롯이 가리키는 인벤토리 인덱스를 서로 맞바꿈 (구 SetHotbarItem 두 번 호출하던 자리 대체)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapHotbarSlots(int32 HotbarIndexA, int32 HotbarIndexB);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetHotbarSlot(int32 HotbarIndex, int32 InventoryIndex);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UItemSubsystem* GetItemSubsystem() const;

	FItemAddResult HandleNoneStackableItems(FName ItemID, int32 RequestedAddAmount);
	int32 HandleStackableItems(FName ItemID, int32 RequestedAddAmount);

	// 서버 로직 전용. CachedItemData(클라 표시용 캐시)는 절대 참조하지 않음
	int32 CalculateAmountForFullStack(int32 CurrentQuantity, int32 MaxStackSize, int32 InitialRequestedAmount) const;

	// 빈 슬롯(FoundIndex)에 새 스택을 채워 넣고 Dirty 마킹
	void AddNewItemAtIndex(int32 SlotIndex, FName ItemID, int32 AmountToAdd);

	// 인벤토리 슬롯이 완전히 비워질 때, 그 슬롯을 참조하던 핫바 슬롯들을 -1로 초기화
	void RemoveItemFromHotbarIfPresent(int32 InventoryIndex);

	// Property ///////////////////////////////////////////////
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 InventorySlotsCapacity = 20;

	UPROPERTY(Replicated)
	FInventoryList InventoryContents;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 HotBarSlotsCapacity = 9;

	// 슬롯 배열. 각 원소는 InventoryContents 몇 번 인덱스를 가리키는지 (-1 = 비어있음)
	UPROPERTY(Replicated)
	TArray<int32> HotbarToInventoryIndex;
};