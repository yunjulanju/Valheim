// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuickSlotBar.generated.h"

class AArcher;
class UInventoryComponent;
class UHorizontalBox;
class UHotBarSlot;

/**
 * 핫바(HotbarContents)와 동일한 내용을 항상 화면에 보여주는 퀵슬롯 바.
 * MainWidget(인벤토리, Tab으로 토글)이 아니라 CharacterWidget(상시 표시)에 넣는다.
 * InventoryPannel의 RefreshHotbar()와 완전히 같은 데이터(InventoryComponent::HotbarContents)를
 * 그대로 다시 그려서 보여줄 뿐이라, 인벤토리 창을 열든 닫든 항상 서버 상태와 동기화된다.
 */
UCLASS()
class VALHEIM_API UQuickSlotBar : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void RefreshQuickSlots();

	// 퀵슬롯 칸을 담을 컨테이너. WBP에서 이름을 QuickSlotBox로 맞춰서 바인딩.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> QuickSlotBox;

	// InventoryPannel에서 쓰는 것과 같은 UHotBarSlot 위젯(WBP_HotBarSlot)을 그대로 지정.
	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	TSubclassOf<UHotBarSlot> HotbarSlotClass;

	// InventoryComponent::HotBarSlotsCapacity 와 동일하게 맞춰줄 것.
	UPROPERTY(EditDefaultsOnly, Category = "QuickSlot")
	int32 QuickSlotCount = 9;

protected:
	UPROPERTY()
	TObjectPtr<AArcher> PlayerCharacter;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryReference;

	UPROPERTY()
	TArray<TObjectPtr<UHotBarSlot>> QuickSlots;
};