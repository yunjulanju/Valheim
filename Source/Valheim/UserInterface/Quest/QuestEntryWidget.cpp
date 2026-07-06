// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Quest/QuestEntryWidget.h"
#include "Components/TextBlock.h"
#include <Data/QuestDataStruct.h>

void UQuestEntryWidget::NativeOnInitialized()
{
}

void UQuestEntryWidget::SetInfo(FText InTitle, FText InDescription, int32 InCurrentAmount, int32 InRequiredAmount, EQuestStatus InStatus)
{
	if (!Title || !Description || !Amount || !SuccessText) return;

	Title->SetText(InTitle);

	if (InStatus == EQuestStatus::ReadyToComplete)
	{
		SuccessText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SuccessText->SetText(FText::FromString(TEXT("Go to NPC")));

		Description->SetVisibility(ESlateVisibility::Collapsed);
		Amount->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SuccessText->SetVisibility(ESlateVisibility::Collapsed);

		Description->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Amount->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		Description->SetText(InDescription);

		FText ProgressString = FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FText::AsNumber(InCurrentAmount),
			FText::AsNumber(InRequiredAmount)
		);
		Amount->SetText(ProgressString);
	}
}

