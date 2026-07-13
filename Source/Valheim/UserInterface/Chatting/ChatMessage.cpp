// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Chatting/ChatMessage.h"
#include "Components/TextBlock.h"

void UChatMessage::SetMessage(const FString& UserId, const FString& Message)
{
	if (UserNameText)
	{
		UserNameText->SetText(FText::FromString(UserId));
	}

	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}
}
