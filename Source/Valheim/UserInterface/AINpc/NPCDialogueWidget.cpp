// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/AINpc/NPCDialogueWidget.h"
#include "UserInterface/Chat/ChatEntry.h"
#include "AINPC/AINpcSubsystem.h"
#include "Core/ValheimGI.h"

#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

static constexpr float NPCDialogueTimeoutSeconds = 10.f;

void UNPCDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DialogueInputBox)
	{
		DialogueInputBox->OnTextCommitted.AddDynamic(this, &UNPCDialogueWidget::OnDialogueInputCommitted);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			Subsystem->OnAIChatReply.AddDynamic(this, &UNPCDialogueWidget::HandleAIChatReply);
			Subsystem->OnAIChatError.AddDynamic(this, &UNPCDialogueWidget::HandleAIChatError);
		}
	}
}

FReply UNPCDialogueWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::X)
	{
		CloseDialogue();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UNPCDialogueWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			Subsystem->OnAIChatReply.RemoveDynamic(this, &UNPCDialogueWidget::HandleAIChatReply);
			Subsystem->OnAIChatError.RemoveDynamic(this, &UNPCDialogueWidget::HandleAIChatError);
		}
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	Super::NativeDestruct();
}

void UNPCDialogueWidget::OpenDialogue(const FString& InNpcId, const FString& InNpcDisplayName)
{
	NpcId = InNpcId;
	NpcDisplayName = InNpcDisplayName;
	SetWaitingForReply(false);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>())
		{
			if (UValheimGI* VGI = Cast<UValheimGI>(GameInstance))
			{
				Subsystem->Connect(VGI->AIServerHost, VGI->AIServerPort);
			}
		}
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		if (DialogueInputBox)
		{
			InputMode.SetWidgetToFocus(DialogueInputBox->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	if (DialogueInputBox)
	{
		DialogueInputBox->SetKeyboardFocus();
	}
}

void UNPCDialogueWidget::CloseDialogue()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}
	SetWaitingForReply(false);

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
	}
	UWidgetBlueprintLibrary::SetFocusToGameViewport();

	RemoveFromParent();
}

void UNPCDialogueWidget::SendDialogueMessage()
{
	if (!DialogueInputBox || bWaitingForReply)
	{
		return;
	}

	const FString Message = DialogueInputBox->GetText().ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UAINpcSubsystem* Subsystem = GameInstance->GetSubsystem<UAINpcSubsystem>();
	UValheimGI* VGI = Cast<UValheimGI>(GameInstance);
	if (!Subsystem || !VGI)
	{
		return;
	}

	if (!Subsystem->IsAIServerConnected())
	{
		AddDialogueEntry(NpcDisplayName, TEXT("지금�? ?�?�할 ???�다."));
		DialogueInputBox->SetText(FText::GetEmpty());
		return;
	}

	const FString PlayerId = VGI->SavedNickname;
	AddDialogueEntry(PlayerId, Message);
	DialogueInputBox->SetText(FText::GetEmpty());

	Subsystem->RequestChat(PlayerId, NpcId, Message);
	SetWaitingForReply(true);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimeoutTimerHandle, FTimerDelegate::CreateUObject(this, &UNPCDialogueWidget::HandleTimeout), NPCDialogueTimeoutSeconds, false);
	}
}

void UNPCDialogueWidget::OnDialogueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	SendDialogueMessage();

	if (DialogueInputBox)
	{
		DialogueInputBox->SetKeyboardFocus();
	}
}

void UNPCDialogueWidget::HandleAIChatReply(const FString& PlayerId, const FString& InNpcId, const FString& Reply)
{
	if (InNpcId != NpcId || !bWaitingForReply)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	AddDialogueEntry(NpcDisplayName, Reply);
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::HandleAIChatError(const FString& PlayerId, const FString& InNpcId, const FString& ErrorCode)
{
	if (InNpcId != NpcId || !bWaitingForReply)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimeoutTimerHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("UNPCDialogueWidget: AI chat error [%s]"), *ErrorCode);
	AddDialogueEntry(NpcDisplayName, TEXT("지금�? ?�?�할 ???�다."));
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::HandleTimeout()
{
	if (!bWaitingForReply)
	{
		return;
	}

	AddDialogueEntry(NpcDisplayName, TEXT("지금�? ?�?�할 ???�다."));
	SetWaitingForReply(false);
}

void UNPCDialogueWidget::AddDialogueEntry(const FString& Speaker, const FString& Message)
{
	if (!DialogueScrollBox || !DialogueEntryWidgetClass)
	{
		return;
	}

	UChatEntry* NewEntry = CreateWidget<UChatEntry>(this, DialogueEntryWidgetClass);
	if (!NewEntry)
	{
		return;
	}

	NewEntry->SetChatEntry(Speaker, Message);
	DialogueScrollBox->AddChild(NewEntry);
	DialogueScrollBox->ScrollToEnd();
}

void UNPCDialogueWidget::SetWaitingForReply(bool bWaiting)
{
	bWaitingForReply = bWaiting;
	if (DialogueInputBox)
	{
		DialogueInputBox->SetIsEnabled(!bWaiting);
	}
}
