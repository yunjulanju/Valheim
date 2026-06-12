// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

class UItemDataBase;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VALHEIM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	FOnInventoryUpdated OnInventoryUpdated;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UFUNCTION()
	UItemDataBase* FindMatchingItem(UItemDataBase* ItemIn) const;

	UFUNCTION()
	UItemDataBase* FindNextItemByID(UItemDataBase* ItemIn) const;

	UFUNCTION()
	UItemDataBase* FindNextPartialStack(UItemDataBase* ItemIn) const;

	UFUNCTION()
	void RemoveSingleInstanceOfItem(UItemDataBase* ItemIn);

	UFUNCTION()
	int32 RemoveAmountOfItem(UItemDataBase* ItemIn, int32 DesiredAmountToRemove);

	UFUNCTION()
	void SplitExistingStack(UItemDataBase* ItemIn, const int32 AmountToSplit);

	UFUNCTION()
	FORCEINLINE int32 GetSlotsCapacity() const {return InventorySlotsCapacity;};

	UFUNCTION()
	FORCEINLINE TArray<UItemDataBase*> GetInventoryContents() const {return InventoryContents;};

	UFUNCTION()
	FORCEINLINE void SetSlotsCapacity(const int32 NewSlotsCapacity) {InventorySlotsCapacity = NewSlotsCapacity;};

	UFUNCTION()
	int32 HandleStackableItems(UItemDataBase* ExistingItem, int32 RequestedAddAmount);

	UFUNCTION()
	int32 CalculateNumberForFullStack(UItemDataBase* ExistingItem, int32 InitialRequestedAddAmount);

	UFUNCTION()
	void AddNewItem(UItemDataBase* Item, int32 AmountToAdd);



	// Property ///////////////////////////////////////////////
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	int32 InventorySlotsCapacity;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UItemDataBase>> InventoryContents;
};
