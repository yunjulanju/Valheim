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

void AArcherHUD::ToggleInventoryPannel()
{
    if (!MainWidget)
    {
        return;
    }

    if (MainWidget->IsVisible())
    {
        MainWidget->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        MainWidget->SetVisibility(ESlateVisibility::Visible);
    }
}
