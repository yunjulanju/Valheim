// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <Data/QuestDataStruct.h>
#include "QuestEntryWidget.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class VALHEIM_API UQuestEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Title;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Description;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Amount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SuccessText;

	void SetInfo(FText InTitle, FText InDescription, int32 InCurrentAmount, int32 InRequiredAmount, EQuestStatus InStatus);
};
