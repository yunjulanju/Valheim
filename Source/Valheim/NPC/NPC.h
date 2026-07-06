// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface/Interactable.h"
#include "GameFramework/Character.h"
#include "NPC.generated.h"


class UQuestSubsystem;
class AArcherPS;
class UWidgetComponent;
UCLASS()
class VALHEIM_API ANPC : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	ANPC();

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName NPCID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* QuestMarkerWidget;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateQuestMarker();

	virtual void Interact(APawn* Interactor) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<FName> CachedQuestIDs;

	void HandleQuestInteraction(class AArcherPS* QuestPlayerState);

	const UQuestSubsystem* QuestSubsystem;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
