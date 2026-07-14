// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterWidget.generated.h"

/**
 * 
 */
class UCharacterHP;
class UQuickSlotBar;
class UChatBox;
UCLASS()
class VALHEIM_API UCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UCharacterHP* CharacterHPWidget;

	UPROPERTY(meta = (BindWidget))
	UQuickSlotBar* QuickSlotBarWidget;

	UPROPERTY(meta = (BindWidget))
	UChatBox* ChatBoxWidget;
	
};
