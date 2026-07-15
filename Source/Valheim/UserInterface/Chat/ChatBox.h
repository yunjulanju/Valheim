// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

/**
 * 
 */
class UEditableText;
class UButton;
class UScrollBox;
class UChatEntry;
UCLASS()
class VALHEIM_API UChatBox : public UUserWidget
{
	GENERATED_BODY()


public:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> ChatInputBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chat")
	TSubclassOf<UChatEntry> ChatEntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString LocalUserId = TEXT("Player");

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ActivateChatInput();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void DeactivateChatInput();

	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsChatInputActive() const { return bIsChatInputActive; }

protected:

	bool bIsChatInputActive = false;

	UFUNCTION()
	void HandleChatReceived(const FString& UserId, const FString& Message);

	UFUNCTION()
	void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void AddChatEntry(const FString& UserId, const FString& Message);
};
