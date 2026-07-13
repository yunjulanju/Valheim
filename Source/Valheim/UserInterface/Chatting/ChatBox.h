// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

/**
 * 
 */
class UVerticalBox;
class UScrollBox;
class UEditableText;
class UChatMessage;
class UTCPClientSubsystem;
UCLASS()
class VALHEIM_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* MessageScrollBox;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* MessageVerticalBox;

	UPROPERTY(meta = (BindWidget))
	UEditableText* ChatInputBox;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UChatMessage> ChatMessageEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	int32 MaxMessageCount = 200;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	FString LocalUserId = TEXT("User");

private:
	UFUNCTION()
	void HandleChatReceived(const FString& UserId, const FString& Message);

	UFUNCTION()
	void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void SendChatMessage(const FString& Message);
	void AddMessageEntry(const FString& UserId, const FString& Message);

	UTCPClientSubsystem* GetTCP() const;
	
};
