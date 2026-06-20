// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

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
	{};
	
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

class UItemDataBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VALHEIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	FOnInventoryUpdated OnInventoryUpdated;

	// Called when the game starts
	virtual void BeginPlay() override;
	UFUNCTION()
	UItemDataBase* FindMatchingItem(UItemDataBase* ItemIn) const;

	UFUNCTION()
	UItemDataBase* FindNextItemByID(UItemDataBase* ItemIn) const;

	UFUNCTION()
	UItemDataBase* FindNextPartialStack(UItemDataBase* ItemIn) const;

	UFUNCTION()
	void RemoveSingleInstanceOfItem(UItemDataBase* ItemToRemove);

	UFUNCTION()
	int32 RemoveAmountOfItem(UItemDataBase* ItemIn, int32 DesiredAmountToRemove);

	UFUNCTION()
	void SplitExistingStack(UItemDataBase* ItemIn, const int32 AmountToSplit);

	UFUNCTION()
	FORCEINLINE TArray<UItemDataBase*> GetHotBarContents() const { return HotBarContents; };

	UFUNCTION()
	FORCEINLINE UItemDataBase* GetHotbarItem(int32 index) { return HotBarContents.IsValidIndex(index) ? HotBarContents[index] : nullptr;};

	UFUNCTION()
	void SetHotbarItem(int32 index, UItemDataBase* Item);

	UFUNCTION()
	FORCEINLINE int32 GetSlotsCapacity() const {return InventorySlotsCapacity;};
	
	UFUNCTION()
	bool MoveInventoryItem(int32 FromIndex, int32 ToIndex);

	UFUNCTION()
	FORCEINLINE UItemDataBase* GetInventoryItem(int32 index) const { return InventoryContents.IsValidIndex(index) ? InventoryContents[index] : nullptr; }

	UFUNCTION()
	FORCEINLINE TArray<UItemDataBase*> GetInventoryContents() const {return InventoryContents;};

	UFUNCTION()
	FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity) {InventorySlotsCapacity = NewSlotsCapacity;};

	UFUNCTION()
	FItemAddResult HandleAddItem(UItemDataBase* InputItem);

	UFUNCTION()
	FItemAddResult HandleNoneStackableItems(UItemDataBase* InputItem, int32 RequestedAddAmount);

	UFUNCTION()
	int32 HandleStackableItems(UItemDataBase* InputItem, int32 RequestedAddAmount);
	UFUNCTION()
	int32 GetUsedSlotCount() const;

	UFUNCTION()
	int32 CalculateNumberForFullStack(UItemDataBase* StackableItem, int32 InitialRequestedAddAmount);

	UFUNCTION()
	void AddNewItem(UItemDataBase* Item, int32 AmountToAdd);

	UFUNCTION()
	void RemoveItemFromInventoryOnly(UItemDataBase* ItemToRemove);

	UFUNCTION()
	void RemoveItemFromHotbarIfPresent(UItemDataBase* ItemToCheck);

	UFUNCTION()
	bool MoveItemToHotbar(UItemDataBase* ItemIn, int32 HotbarIndex);

	UFUNCTION()
	bool MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex);

protected:
	// Property ///////////////////////////////////////////////
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 InventorySlotsCapacity;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UItemDataBase>> InventoryContents;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 HotBarSlotsCapacity = 9;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UItemDataBase>> HotBarContents;
};
