// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Chat/ChatEntry.h"
#include "Components/TextBlock.h"

void UChatEntry::SetChatEntry(const FString& InUserId, const FString& InMessage)
{
	if (UserIdText)
	{
		UserIdText->SetText(FText::FromString(InUserId + TEXT(":")));
	}

	if (MessageText)
	{
		MessageText->SetText(FText::FromString(InMessage));
	}
}
