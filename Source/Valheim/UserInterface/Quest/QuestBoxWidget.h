// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestBoxWidget.generated.h"

/**
 * 
 */
class UVerticalBox;
class AArcherPS;
class UQuestSubsystem;
class UQuestEntryWidget;
UCLASS()
class VALHEIM_API UQuestBoxWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void UpdateQuestList();



	UPROPERTY(meta = (BindWidget))
	UVerticalBox* QuestList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UQuestEntryWidget> QuestEntryWidgetClass;

	AArcherPS* ArcherPS;

	const UQuestSubsystem* QuestSubsystem;
};
