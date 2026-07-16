// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Chat/ChatBox.h"
#include "UserInterface/Chat/ChatEntry.h"

#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"

#include "TCP/TCPClientSubsystem.h"

void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UChatBox::OnChatInputCommitted);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UTCPClientSubsystem* TCPSubsystem = GameInstance->GetSubsystem<UTCPClientSubsystem>())
		{
			TCPSubsystem->OnChatReceived.AddDynamic(this, &UChatBox::HandleChatReceived);
		}
	}
}

void UChatBox::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UTCPClientSubsystem* TCPSubsystem = GameInstance->GetSubsystem<UTCPClientSubsystem>())
		{
			TCPSubsystem->OnChatReceived.RemoveDynamic(this, &UChatBox::HandleChatReceived);
		}
	}

	Super::NativeDestruct();
}

void UChatBox::SendChatMessage()
{
	if (!ChatInputBox)
	{
		return;
	}

	const FString Message = ChatInputBox->GetText().ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UTCPClientSubsystem* TCPSubsystem = GameInstance->GetSubsystem<UTCPClientSubsystem>())
		{
			TCPSubsystem->SendChat(LocalUserId, Message);
		}
	}

	ChatInputBox->SetText(FText::GetEmpty());

	ChatInputBox->SetKeyboardFocus();
}

void UChatBox::ActivateChatInput()
{
	if (!ChatInputBox)
	{
		return;
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(ChatInputBox->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	ChatInputBox->SetKeyboardFocus();
	bIsChatInputActive = true;
}

void UChatBox::DeactivateChatInput()
{
	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
	}

	bIsChatInputActive = false;
}

void UChatBox::HandleChatReceived(const FString& UserId, const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("ChatBox::HandleChatReceived [%s]: %s"), *UserId, *Message);
	AddChatEntry(UserId, Message);
}

void UChatBox::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	const FString Message = Text.ToString().TrimStartAndEnd();
	if (Message.IsEmpty())
	{
		DeactivateChatInput();
	}
	else
	{
		SendChatMessage();
	}
}

void UChatBox::AddChatEntry(const FString& UserId, const FString& Message)
{
	if (!ChatScrollBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatBox::AddChatEntry aborted: ChatScrollBox is null (binding failed)."));
		return;
	}

	if (!ChatEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatBox::AddChatEntry aborted: ChatEntryWidgetClass is not set in Class Defaults."));
		return;
	}

	UChatEntry* NewEntry = CreateWidget<UChatEntry>(this, ChatEntryWidgetClass);
	if (!NewEntry)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatBox::AddChatEntry aborted: CreateWidget<UChatEntry> failed."));
		return;
	}

	NewEntry->SetChatEntry(UserId, Message);
	ChatScrollBox->AddChild(NewEntry);
	ChatScrollBox->ScrollToEnd();
	UE_LOG(LogTemp, Warning, TEXT("ChatBox::AddChatEntry added entry [%s]: %s"), *UserId, *Message);
}