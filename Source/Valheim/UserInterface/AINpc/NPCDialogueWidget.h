// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NPCDialogueWidget.generated.h"

class UEditableText;
class UScrollBox;
class UChatEntry;

UCLASS()
class VALHEIM_API UNPCDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> DialogueInputBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> DialogueScrollBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AINpc")
	TSubclassOf<UChatEntry> DialogueEntryWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void OpenDialogue(const FString& InNpcId, const FString& InNpcDisplayName);

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void CloseDialogue();

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void SendDialogueMessage();

protected:
	FString NpcId;
	FString NpcDisplayName;
	bool bWaitingForReply = false;
	FTimerHandle TimeoutTimerHandle;

	UFUNCTION()
	void OnDialogueInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleAIChatReply(const FString& PlayerId, const FString& InNpcId, const FString& Reply);

	UFUNCTION()
	void HandleAIChatError(const FString& PlayerId, const FString& InNpcId, const FString& ErrorCode);

	void HandleTimeout();
	void AddDialogueEntry(const FString& Speaker, const FString& Message);
	void SetWaitingForReply(bool bWaiting);
};
