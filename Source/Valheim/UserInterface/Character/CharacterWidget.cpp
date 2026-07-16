// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Character/CharacterWidget.h"
#include "UserInterface/Chat/ChatBox.h"

void UCharacterWidget::ShowChat()
{
	if (ChatBox)
	{
		ChatBox->ActivateChatInput();
	}
}

void UCharacterWidget::HideChat()
{
	if (ChatBox)
	{
		ChatBox->DeactivateChatInput();
	}
}

void UCharacterWidget::ToggleChat()
{
	if (!ChatBox)
	{
		return;
	}

	if (ChatBox->IsChatInputActive())
	{
		ChatBox->DeactivateChatInput();
	}
	else
	{
		ChatBox->ActivateChatInput();
	}
}

bool UCharacterWidget::IsChatActive() const
{
	return ChatBox && ChatBox->IsChatInputActive();
}
