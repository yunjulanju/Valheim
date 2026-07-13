// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/ArcherHUD.h"
#include "UserInterface/MainWidget.h"

void AArcherHUD::BeginPlay()
{
	Super::BeginPlay();

    if (MainWidgetClass)
    {
        MainWidget = CreateWidget<UMainWidget>(GetWorld(), MainWidgetClass);
        if (MainWidget)
        {
            MainWidget->AddToViewport(5);
            MainWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    if (CharacterWidgetClass)
    {  
        CharacterWidget = CreateWidget<UUserWidget>(GetWorld(), CharacterWidgetClass);
        if (CharacterWidget)
        {
            CharacterWidget->AddToViewport();
            CharacterWidget->SetVisibility(ESlateVisibility::Visible);
        }
    }
    if (ChatWidgetClass)
    {
        ChatWidget = CreateWidget<UUserWidget>(GetWorld(), ChatWidgetClass);
        if (ChatWidget)
        {
            ChatWidget->AddToViewport(2);
            ChatWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void AArcherHUD::ToggleMainWidget()
{
    if (!MainWidget)
    {
        return;
    }

    if (bIsMenuVisible)
    {
        bIsMenuVisible = false;
        MainWidget->SetVisibility(ESlateVisibility::Hidden);
        const FInputModeGameOnly InputMode;
        GetOwningPlayerController()->SetInputMode(InputMode);
        GetOwningPlayerController()->SetShowMouseCursor(false);
    }
    else
    {
        bIsMenuVisible = true;
        MainWidget->SetVisibility(ESlateVisibility::Visible);
        const FInputModeGameAndUI InputMode;
        GetOwningPlayerController()->SetInputMode(InputMode);
        GetOwningPlayerController()->SetShowMouseCursor(true);
    }
}

void AArcherHUD::ToggleChatWidget()
{
    if (!ChatWidget)
    {
        return;
    }

    if (bIsChatVisible)
    {
        bIsChatVisible = false;
        ChatWidget->SetVisibility(ESlateVisibility::Hidden);
        const FInputModeGameOnly InputMode;
        GetOwningPlayerController()->SetInputMode(InputMode);
    }
    else
    {
        bIsChatVisible = true;
        ChatWidget->SetVisibility(ESlateVisibility::Visible);
        const FInputModeGameAndUI InputMode;
        GetOwningPlayerController()->SetInputMode(InputMode);
    }
}