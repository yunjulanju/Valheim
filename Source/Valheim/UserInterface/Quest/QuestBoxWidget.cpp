// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Quest/QuestBoxWidget.h"
#include "Components/VerticalBox.h"
#include "Character/ArcherPS.h"
#include "Quest/QuestSubsystem.h"
#include "Engine/GameInstance.h"
#include "UserInterface/Quest/QuestEntryWidget.h"

void UQuestBoxWidget::NativeOnInitialized()
{

}

void UQuestBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetGameInstance())
	{
		QuestSubsystem = GetGameInstance()->GetSubsystem<UQuestSubsystem>();
	}

	TryBindArcherPS();

	UpdateQuestList();
}

void UQuestBoxWidget::NativeDestruct()
{
	if (ArcherPS)
	{
		ArcherPS->OnActiveQuestsChanged.RemoveDynamic(this, &UQuestBoxWidget::UpdateQuestList);
	}

	Super::NativeDestruct();
}

void UQuestBoxWidget::TryBindArcherPS()
{
	ArcherPS = GetOwningPlayerState<AArcherPS>();
	if (ArcherPS)
	{
		ArcherPS->OnActiveQuestsChanged.AddDynamic(this, &UQuestBoxWidget::UpdateQuestList);
		UpdateQuestList();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &UQuestBoxWidget::TryBindArcherPS);
	}
}

void UQuestBoxWidget::UpdateQuestList()
{
	if (ArcherPS)
	{
		QuestList->ClearChildren();
		TArray<FActiveQuest> Quests = ArcherPS->GetActiveQuests();
		if (Quests.Num() == 0)
		{
			return;
		}

		if (!QuestEntryWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("UQuestBoxWidget::UpdateQuestList() !QuestEntryWidgetClass"))
			return;
		}
	
		for (FActiveQuest Quest : Quests)
		{
			UQuestEntryWidget* QuestEntryWidget = CreateWidget<UQuestEntryWidget>(this, QuestEntryWidgetClass);
			if (QuestEntryWidget)
			{
				QuestList->AddChild(QuestEntryWidget);
				FQuestData QuestData;
				if (QuestSubsystem->GetQuestData(Quest.QuestID, QuestData))
				{
					QuestEntryWidget->SetInfo(
						QuestData.Title,
						QuestData.Description,
						Quest.CurrentAmount,
						QuestData.RequiredAmount,
						Quest.Status
					);
				}
			}
		}
	}
}
