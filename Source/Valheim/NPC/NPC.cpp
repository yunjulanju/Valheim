// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPC.h"
#include "Quest/QuestSubsystem.h"
#include "Character/ArcherPS.h"
#include <Character/Archer.h>
#include "Components/WidgetComponent.h"

ANPC::ANPC()
{
	PrimaryActorTick.bCanEverTick = true;

	QuestMarkerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("QuestMarkerWidget"));
	QuestMarkerWidget->SetupAttachment(RootComponent);
	QuestMarkerWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
}

void ANPC::BeginPlay()
{
	Super::BeginPlay();

	if (GetGameInstance())
	{
		QuestSubsystem = GetGameInstance()->GetSubsystem<UQuestSubsystem>();
	}

	CachedQuestIDs = QuestSubsystem->GetQuestsByGiver(NPCID);
	UpdateQuestMarker();
}

void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANPC::UpdateQuestMarker()
{
	if (!QuestMarkerWidget) return;

	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC || !LocalPC->IsLocalController()) return;

	AArcherPS* LocalPS = LocalPC->GetPlayerState<AArcherPS>();
	if (!LocalPS)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ANPC::UpdateQuestMarker);
		return;
	}
	LocalPS->OnActiveQuestsChanged.AddDynamic(this, &ANPC::UpdateQuestMarker);


	bool bShouldShowWidget = false;

	for (const FName& QuestID : CachedQuestIDs)
	{
		if (LocalPS->IsQuestCompleted(QuestID))
		{
			continue;
		}

		FActiveQuest ActiveQuest;
		if (LocalPS->GetActiveQuest(QuestID, ActiveQuest))
		{
			if (ActiveQuest.Status == EQuestStatus::ReadyToComplete)
			{
				bShouldShowWidget = true;
				break;
			}
			continue;
		}

		bShouldShowWidget = true;
		break;
	}

	QuestMarkerWidget->SetVisibility(bShouldShowWidget);
}

void ANPC::Interact(APawn* Interactor)
{
	if (!Interactor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !Interactor"))
		return;
	}

	AArcher* Taker = Cast<AArcher>(Interactor);
	if (!Taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !Taker"))
		return;
	}

	AArcherPS* QuestPlayerState = Taker->GetPlayerState<AArcherPS>();
	if (!QuestPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::Interact !QuestPlayerState"))
		return;
	}

	HandleQuestInteraction(QuestPlayerState);
}

void ANPC::HandleQuestInteraction(AArcherPS* QuestPlayerState)
{
	if (!QuestPlayerState)
	{
		UE_LOG(LogTemp, Warning, TEXT("ANPC::HandleQuestInteraction !QuestPlayerState"))
		return;
	}

	for (const FName& QuestID : CachedQuestIDs)
	{
		if (QuestPlayerState->IsQuestCompleted(QuestID))
		{
			continue;
		}

		FActiveQuest ActiveQuest;
		if (QuestPlayerState->GetActiveQuest(QuestID, ActiveQuest))
		{
			if (ActiveQuest.Status == EQuestStatus::ReadyToComplete)
			{
				QuestPlayerState->CompleteQuest(QuestID);
			}
			return;
		}

		QuestPlayerState->AcceptQuest(QuestID);
		return;
	}
}