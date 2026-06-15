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
            MainWidget->AddToViewport();
            MainWidget->SetVisibility(ESlateVisibility::Hidden);
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