// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ConsumableItemDataBase.h"
#include "Character/Archer.h"

void UConsumableItemDataBase::Use(AArcher* User)
{
	if (ItemCategory.ItemCategory != EItemCategory::Consumable || !User)
	{
		return;
	}

	switch (ItemCategory.ItemType)
	{
	case EItemType::Heal:
		User->AddHP(ItemCategory.Value);
		break;
	case EItemType::Damage:
		//우선 데미지 입는 거로 햇는데, 공격량 증가하는 것도 괜찮을 수도
		User->AddHP(-1*ItemCategory.Value);
		break;
	case EItemType::Defense:
		//데미지 방어 등
		break;
	default:
		break;
	}
}
