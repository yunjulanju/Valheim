// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Chatting/ChatBox.h"
#include "UserInterface/Chatting/ChatMessage.h"
#include "TCP/TCPClientSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"

void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	if (UTCPClientSubsystem* TCP = GetTCP())
	{
		TCP->OnChatReceived.AddDynamic(this, &UChatBox::HandleChatReceived);
	}

	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.AddDynamic(this, &UChatBox::OnChatInputCommitted);
	}
}

void UChatBox::NativeDestruct()
{
	if (UTCPClientSubsystem* TCP = GetTCP())
	{
		TCP->OnChatReceived.RemoveDynamic(this, &UChatBox::HandleChatReceived);
	}

	if (ChatInputBox)
	{
		ChatInputBox->OnTextCommitted.RemoveDynamic(this, &UChatBox::OnChatInputCommitted);
	}

	Super::NativeDestruct();
}

void UChatBox::HandleChatReceived(const FString& UserId, const FString& Message)
{
	AddMessageEntry(UserId, Message);
}

void UChatBox::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}

	SendChatMessage(Text.ToString());
}

void UChatBox::SendChatMessage(const FString& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (UTCPClientSubsystem* TCP = GetTCP())
	{
		TCP->SendChat(LocalUserId, Message);
	}

	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());
	}
}

void UChatBox::AddMessageEntry(const FString& UserId, const FString& Message)
{
	if (!ChatMessageEntryClass || !MessageVerticalBox)
	{
		return;
	}

	UChatMessage* Entry = CreateWidget<UChatMessage>(this, ChatMessageEntryClass);
	if (!Entry)
	{
		return;
	}

	Entry->SetMessage(UserId, Message);
	MessageVerticalBox->AddChildToVerticalBox(Entry);

	if (MaxMessageCount > 0 && MessageVerticalBox->GetChildrenCount() > MaxMessageCount)
	{
		MessageVerticalBox->RemoveChildAt(0);
	}

	if (MessageScrollBox)
	{
		MessageScrollBox->ScrollToEnd();
	}
}

UTCPClientSubsystem* UChatBox::GetTCP() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UTCPClientSubsystem>();
	}

	return nullptr;
}
