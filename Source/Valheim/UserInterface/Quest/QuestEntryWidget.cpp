// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Quest/QuestEntryWidget.h"
#include "Components/TextBlock.h"

void UQuestEntryWidget::NativeOnInitialized()
{
}

void UQuestEntryWidget::SetInfo(FText InTitle, FText InDescription, int32 InAmount)
{
	if (InTitle.IsEmpty() || InDescription.IsEmpty())
	{
		return;
	}
	if (Title && Description && Amount)
	{
		Title->SetText(InTitle);
		Description->SetText(InDescription);

		/*const FText ProgressString = FText::Format(
			NSLOCTEXT("Quest", "ProgressFormat", "{0}/{1}"),
			FText::AsNumber(CurrentAmount),
			FText::AsNumber(RequiredAmount)
		);*/

		Amount->SetText(FText::AsNumber(InAmount));
	}
	
}
