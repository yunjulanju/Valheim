// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatEntry.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class VALHEIM_API UChatEntry : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> UserIdText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MessageText;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetChatEntry(const FString& InUserId, const FString& InMessage);
	
};
