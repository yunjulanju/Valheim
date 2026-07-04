// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 아이템을 UObject 포인터가 아니라 "어느 인벤토리의 몇 번 슬롯인지"로 식별한다.
 * (FInventoryItemInstance는 값 타입이라 드래그 오퍼레이션이 그 자체를 들고 있을 이유가 없음)
 */
class UInventoryComponent;
UCLASS()
class VALHEIM_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	UInventoryComponent* SourceInventory;

	UPROPERTY(VisibleAnywhere)
	bool bFromHotbar = false;

	UPROPERTY(VisibleAnywhere)
	int32 SourceHotbarIndex = -1;

	UPROPERTY(VisibleAnywhere)
	int32 SourceInventoryIndex = -1;
};
