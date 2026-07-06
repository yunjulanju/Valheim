// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUser.generated.h"

class UTextBlock;
class UEditableTextBox;
class UButton;
class ALobbyPlayerState;
UCLASS()
class VALHEIM_API ULobbyUser : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetInfo(ALobbyPlayerState* InPlayerState);

	UFUNCTION(BlueprintCallable)
	void RefreshUI();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Ready; // 레디 텍스트 표시하는 녀석

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Editable_PlayerName; // 닉네임 기입하는 박스 표시하는 녀석

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> Btn_Ready; // 레디버튼 표시하는 녀석

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> Btn_KickPlayer; // 강제추방버튼 표시하는 녀석

protected:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<ALobbyPlayerState> PlayerState;

};
