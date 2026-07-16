// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterWidget.generated.h"

/**
 * 
 */
class UChatBox;
UCLASS()
class VALHEIM_API UCharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UChatBox> ChatBox;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ToggleChat();

	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsChatActive() const;
};
