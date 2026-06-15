// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ArcherHUD.generated.h"

/**
 * 
 */
class UMainWidget;
UCLASS()
class VALHEIM_API AArcherHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	void ToggleMainWidget();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY()
	UMainWidget* MainWidget;

	bool bIsMenuVisible = false;

	
};
