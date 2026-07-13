// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMessage.generated.h"

/**
 * 
 */
class UTextBlock;
UCLASS()
class VALHEIM_API UChatMessage : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMessage(const FString& UserId, const FString& Message);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* UserNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
};
