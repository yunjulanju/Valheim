// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestBoxWidget.generated.h"

/**
 * 
 */
class UVerticalBox;
UCLASS()
class VALHEIM_API UQuestBoxWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* QuestList;

	void UpdateList();
};
