// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterHP.generated.h"

/**
 * 
 */
class AArcher;
class UTextBlock;
class UProgressBar;
UCLASS()
class VALHEIM_API UCharacterHP : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void UpdateHP();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPBar;

	UPROPERTY()
	AArcher* PlayerCharacter;

	
};
